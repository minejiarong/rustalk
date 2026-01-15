use crate::net::protocol::Outgoing;
use tokio::sync::mpsc::Sender;
use tokio::time::{interval, Duration};

pub async fn start(tx: Sender<Outgoing>) {
    let mut ticker = interval(Duration::from_secs(30));
    loop {
        ticker.tick().await;
        let _ = tx.try_send(Outgoing::Ping);
    }
}
