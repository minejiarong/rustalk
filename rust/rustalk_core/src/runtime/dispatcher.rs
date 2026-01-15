use tokio::sync::mpsc::Receiver;
use crate::domain::message::Message;

pub enum CoreEvent {
    SendMessage(Message),
    IncomingMessage(Message),
    Shutdown,
}

pub async fn run(mut rx: Receiver<CoreEvent>) {
    while let Some(ev) = rx.recv().await {
        match ev {
            CoreEvent::SendMessage(msg) | CoreEvent::IncomingMessage(msg) => {
                crate::storage::sqlite::save_message(&msg);
            }
            CoreEvent::Shutdown => {
                break;
            }
        }
    }
}
