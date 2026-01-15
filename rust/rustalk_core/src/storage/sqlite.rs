use once_cell::sync::OnceCell;
use rusqlite::{params, Connection};
use std::sync::Mutex;

use crate::domain::message::Message;

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
    let _ = DB.set(Mutex::new(conn));
}

pub fn save_message(msg: &Message) {
    let conn = DB.get().unwrap().lock().unwrap();
    conn.execute(
        "INSERT INTO messages(from_user, to_user, content, timestamp) VALUES (?1, ?2, ?3, ?4)",
        params![msg.from, msg.to, msg.content, msg.timestamp],
    )
    .unwrap();
}

pub fn load_messages(peer: i64, limit: i32) -> Vec<Message> {
    let conn = DB.get().unwrap().lock().unwrap();
    let mut stmt = conn
        .prepare(
            "SELECT from_user, to_user, content, timestamp
             FROM messages
             WHERE from_user = ?1 OR to_user = ?1
             ORDER BY timestamp DESC
             LIMIT ?2",
        )
        .unwrap();
    let rows = stmt
        .query_map(params![peer, limit], |row| {
            Ok(Message {
                from: row.get(0)?,
                to: row.get(1)?,
                content: row.get(2)?,
                timestamp: row.get(3)?,
            })
        })
        .unwrap();
    let mut out = Vec::new();
    for r in rows {
        out.push(r.unwrap());
    }
    out
}
