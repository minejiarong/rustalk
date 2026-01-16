use once_cell::sync::OnceCell;
use rusqlite::{params, Connection};
use std::sync::Mutex;

use crate::domain::message::Message;
use rusqlite::Error as SqlError;

static DB: OnceCell<Mutex<Connection>> = OnceCell::new();

pub fn init(path: &str) {
    let conn = Connection::open(path).unwrap();
    conn.execute(
        "CREATE TABLE IF NOT EXISTS messages(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            from_user INTEGER NOT NULL,
            to_user INTEGER NOT NULL,
            content TEXT NOT NULL,
            timestamp INTEGER NOT NULL
        )",
        [],
    )
    .unwrap();
    conn.execute(
        "CREATE TABLE IF NOT EXISTS users(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL
        )",
        [],
    )
    .unwrap();
    conn.execute(
        "CREATE TABLE IF NOT EXISTS contacts(
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL
        )",
        [],
    )
    .unwrap();
    let _ = DB.set(Mutex::new(conn));
}

pub fn save_message(msg: &Message) {
    let conn = DB.get().unwrap().lock().unwrap();
    // Check if message already exists (deduplication)
    let count: i64 = conn.query_row(
        "SELECT count(*) FROM messages WHERE from_user=?1 AND to_user=?2 AND timestamp=?3 AND content=?4",
        params![msg.from, msg.to, msg.timestamp, msg.content],
        |row| row.get(0),
    ).unwrap_or(0);

    if count > 0 {
        return;
    }

    conn.execute(
        "INSERT INTO messages(from_user, to_user, content, timestamp) VALUES (?1, ?2, ?3, ?4)",
        params![msg.from, msg.to, msg.content, msg.timestamp],
    )
    .unwrap();
}

pub fn delete_message(id: i64) -> Result<(), SqlError> {
    let conn = DB.get().unwrap().lock().unwrap();
    conn.execute("DELETE FROM messages WHERE id = ?1", params![id])?;
    Ok(())
}

pub fn load_messages(peer: i64, limit: i32) -> Vec<(i64, Message)> {
    let conn = DB.get().unwrap().lock().unwrap();
    let mut stmt = conn
        .prepare(
            "SELECT id, from_user, to_user, content, timestamp
             FROM messages
             WHERE from_user = ?1 OR to_user = ?1
             ORDER BY timestamp DESC
             LIMIT ?2",
        )
        .unwrap();
    let rows = stmt
        .query_map(params![peer, limit], |row| {
            Ok((
                row.get(0)?,
                Message {
                    from: row.get(1)?,
                    to: row.get(2)?,
                    content: row.get(3)?,
                    timestamp: row.get(4)?,
                }
            ))
        })
        .unwrap();
    let mut out = Vec::new();
    for r in rows {
        out.push(r.unwrap());
    }
    out
}

pub fn search_messages(peer: i64, keyword: &str) -> Vec<(i64, Message)> {
    let conn = DB.get().unwrap().lock().unwrap();
    let pattern = format!("%{}%", keyword);
    let mut stmt = conn
        .prepare(
            "SELECT id, from_user, to_user, content, timestamp
             FROM messages
             WHERE (from_user = ?1 OR to_user = ?1) AND content LIKE ?2
             ORDER BY timestamp DESC",
        )
        .unwrap();
    let rows = stmt
        .query_map(params![peer, pattern], |row| {
            Ok((
                row.get(0)?,
                Message {
                    from: row.get(1)?,
                    to: row.get(2)?,
                    content: row.get(3)?,
                    timestamp: row.get(4)?,
                }
            ))
        })
        .unwrap();
    let mut out = Vec::new();
    for r in rows {
        if let Ok(item) = r {
            out.push(item);
        }
    }
    out
}

pub fn create_user(username: &str, password_hash: &str, salt: &str) -> Result<i64, SqlError> {
    let conn = DB.get().unwrap().lock().unwrap();
    conn.execute(
        "INSERT INTO users(username, password_hash, salt) VALUES (?1, ?2, ?3)",
        params![username, password_hash, salt],
    )?;
    let id = conn.last_insert_rowid();
    Ok(id as i64)
}

pub fn find_user(username: &str) -> Result<Option<(i64, String, String)>, SqlError> {
    let conn = DB.get().unwrap().lock().unwrap();
    let mut stmt = conn.prepare(
        "SELECT id, password_hash, salt FROM users WHERE username = ?1 LIMIT 1",
    )?;
    let mut rows = stmt.query(params![username])?;
    if let Some(row) = rows.next()? {
        let id: i64 = row.get(0)?;
        let hash: String = row.get(1)?;
        let salt: String = row.get(2)?;
        Ok(Some((id, hash, salt)))
    } else {
        Ok(None)
    }
}

pub fn upsert_contact(id: i64, name: &str) -> Result<(), SqlError> {
    let conn = DB.get().unwrap().lock().unwrap();
    conn.execute(
        "INSERT INTO contacts(id, name) VALUES(?1, ?2)
         ON CONFLICT(id) DO UPDATE SET name = excluded.name",
        params![id, name],
    )?;
    Ok(())
}

pub fn load_contacts() -> Result<Vec<crate::domain::contact::Contact>, SqlError> {
    let conn = DB.get().unwrap().lock().unwrap();
    let mut stmt = conn.prepare("SELECT id, name FROM contacts ORDER BY name ASC")?;
    let rows = stmt.query_map([], |row| {
        Ok(crate::domain::contact::Contact {
            id: row.get(0)?,
            name: row.get(1)?,
        })
    })?;
    let mut out = Vec::new();
    for r in rows {
        out.push(r?);
    }
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::env;
    use std::fs;

    fn db_path(name: &str) -> String {
        let mut p = env::temp_dir();
        p.push(format!("rustalk_test_{}.db", name));
        p.to_string_lossy().to_string()
    }

    fn reset_db(path: &str) {
        let _ = fs::remove_file(path);
        init(path);
    }

    #[test]
    fn dedup_insert() {
        let path = db_path("dedup");
        reset_db(&path);
        let m = Message { from: 101, to: 102, content: "Hello".into(), timestamp: 1234567890 };
        save_message(&m);
        save_message(&m);
        let rows = load_messages(101, 100);
        assert_eq!(rows.len(), 1);
        assert_eq!(rows[0].1.content, "Hello");
    }

    #[test]
    fn delete_message_works() {
        let path = db_path("delete");
        reset_db(&path);
        let m = Message { from: 201, to: 202, content: "ToDelete".into(), timestamp: 1111 };
        save_message(&m);
        let rows = load_messages(201, 10);
        assert_eq!(rows.len(), 1);
        let id = rows[0].0;
        let _ = delete_message(id);
        let rows2 = load_messages(201, 10);
        assert_eq!(rows2.len(), 0);
    }

    #[test]
    fn search_messages_filters_and_orders() {
        let path = db_path("search");
        reset_db(&path);
        save_message(&Message { from: 301, to: 302, content: "alpha one".into(), timestamp: 10 });
        save_message(&Message { from: 301, to: 302, content: "beta two".into(), timestamp: 20 });
        save_message(&Message { from: 302, to: 301, content: "alpha three".into(), timestamp: 30 });
        let rows = search_messages(301, "alpha");
        assert_eq!(rows.len(), 2);
        assert!(rows[0].1.timestamp >= rows[1].1.timestamp);
        for (_, m) in rows {
            assert!(m.content.contains("alpha"));
        }
    }
}
