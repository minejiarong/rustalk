use tokio::sync::mpsc::Receiver;
use crate::domain::message::Message;

pub enum CoreEvent {
    SendMessage(Message),
    IncomingMessage(Message),
    Connect(String),
    Shutdown,
}

pub async fn run(mut rx: Receiver<CoreEvent>) {
    while let Some(ev) = rx.recv().await {
        match ev {
            CoreEvent::SendMessage(msg) => {
                // 1. 发送给网络
                let _ = crate::net::ws_client::send(msg.clone());
                // 2. 存本地
                crate::storage::sqlite::save_message(&msg);
            }
            CoreEvent::IncomingMessage(msg) => {
                // 收到消息 -> 存库
                crate::storage::sqlite::save_message(&msg);
            }
            CoreEvent::Connect(url) => {
                let _ = crate::net::ws_client::connect(&url).await;
            }
            CoreEvent::Shutdown => {
                break;
            }
        }
    }
}