use crate::domain::message::Message;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize)]
pub struct WireMessage {
    pub from: i64,
    pub to: i64,
    pub content: String,
    pub timestamp: i64,
}

pub enum Outgoing {
    Send(Message),
    Ping,
}

pub fn encode_message(msg: &Message) -> String {
    let wire = WireMessage {
        from: msg.from,
        to: msg.to,
        content: msg.content.clone(),
        timestamp: msg.timestamp,
    };
    serde_json::to_string(&wire).unwrap_or_default()
}

pub fn decode_message(text: &str) -> Option<Message> {
    match serde_json::from_str::<WireMessage>(text) {
        Ok(w) => Some(Message {
            from: w.from,
            to: w.to,
            content: w.content,
            timestamp: w.timestamp,
        }),
        Err(_) => None,
    }
}
