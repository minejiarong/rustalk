use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Clone)]
pub enum MessageType {
    Text,
}

#[derive(Serialize, Deserialize, Clone)]
pub struct Message {
    pub from: i64,
    pub to: i64,
    pub content: String,
    pub timestamp: i64,
}