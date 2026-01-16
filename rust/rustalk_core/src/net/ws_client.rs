use crate::domain::message::Message;
use crate::net::protocol::{decode_message, encode_message, Outgoing};
use crate::runtime::{channels, dispatcher::CoreEvent};
use futures_util::{SinkExt, StreamExt};
use once_cell::sync::OnceCell;
use tokio::sync::mpsc::{self, Sender};
use tokio_tungstenite::tungstenite::{self, Utf8Bytes};
use tokio_tungstenite::connect_async;

static OUT_TX: OnceCell<Sender<Outgoing>> = OnceCell::new();

pub async fn connect(url: &str) -> i32 {
    let ws = match connect_async(url).await {
        Ok((stream, _)) => stream,
        Err(_) => return -1,
    };
    let (mut write, mut read) = ws.split();
    let (tx, mut rx) = mpsc::channel::<Outgoing>(1024);
    let _ = OUT_TX.set(tx.clone());

    tokio::spawn(async move {
        while let Some(out) = rx.recv().await {
            match out {
                Outgoing::Send(msg) => {
                    let text = encode_message(&msg);
                    let _ = write.send(tungstenite::Message::Text(Utf8Bytes::from(text))).await;
                }
                Outgoing::Ping => {
                    let _ = write.send(tungstenite::Message::Ping(Vec::new().into())).await;
                }
            }
        }
    });

    {
        let hb_tx = tx.clone();
        tokio::spawn(crate::net::heartbeat::start(hb_tx));
    }

    tokio::spawn(async move {
        while let Some(item) = read.next().await {
            if let Ok(frame) = item {
                match frame {
                    tungstenite::Message::Text(t) => {
                        if let Some(msg) = decode_message(&t) {
                            let _ = channels::send(CoreEvent::IncomingMessage(msg));
                        }
                    }
                    _ => {}
                }
            }
        }
    });

    0
}

pub fn send(msg: Message) -> i32 {
    match OUT_TX.get() {
        Some(tx) => match tx.try_send(Outgoing::Send(msg)) {
            Ok(_) => 0,
            Err(_) => -1,
        },
        None => -2,
    }
}