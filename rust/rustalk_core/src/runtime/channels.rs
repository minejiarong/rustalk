use once_cell::sync::OnceCell;
use tokio::sync::mpsc::Sender;

use super::dispatcher::CoreEvent;

static CORE_TX: OnceCell<Sender<CoreEvent>> = OnceCell::new();

pub fn set_sender(tx: Sender<CoreEvent>) {
    let _ = CORE_TX.set(tx);
}

pub fn send(event: CoreEvent) -> Result<(), &'static str> {
    match CORE_TX.get() {
        Some(tx) => tx.try_send(event).map_err(|_| "send_failed"),
        None => Err("not_initialized"),
    }
}
