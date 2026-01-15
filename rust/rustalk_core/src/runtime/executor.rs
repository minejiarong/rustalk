use once_cell::sync::OnceCell;
use tokio::runtime::Builder;
use tokio::sync::mpsc;
use super::channels;
use super::dispatcher;

static RUNTIME_STARTED: OnceCell<()> = OnceCell::new();

pub fn init() -> i32 {
    if RUNTIME_STARTED.get().is_some() {
        return 0;
    }

    let (tx, rx) = mpsc::channel(1024);
    channels::set_sender(tx);

    std::thread::spawn(move || {
        let rt = Builder::new_multi_thread().enable_all().build().unwrap();
        rt.block_on(async move {
            dispatcher::run(rx).await;
        });
    });

    let _ = RUNTIME_STARTED.set(());
    0
}
