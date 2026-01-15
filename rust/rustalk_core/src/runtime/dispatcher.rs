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
            CoreEvent::SendMessage(msg) | CoreEvent::IncomingMessage(msg) => {
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
