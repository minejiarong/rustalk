use futures_util::{SinkExt, StreamExt};
use tokio::net::TcpListener;
use tokio_tungstenite::{accept_async, tungstenite::Message};

#[tokio::main]
async fn main() {
    let addr = "127.0.0.1:8080";
    println!("WebSocket server listening on ws://{}", addr);

    let listener = TcpListener::bind(addr).await.expect("bind failed");

    loop {
        match listener.accept().await {
            Ok((stream, peer)) => {
                println!("Incoming connection from {}", peer);
                tokio::spawn(async move {
                    match accept_async(stream).await {
                        Ok(ws_stream) => {
                            let (mut write, mut read) = ws_stream.split();
                            while let Some(msg) = read.next().await {
                                match msg {
                                    Ok(Message::Text(text)) => {
                                        // Echo back received text
                                        let _ = write.send(Message::Text(text)).await;
                                    }
                                    Ok(Message::Binary(bin)) => {
                                        let _ = write.send(Message::Binary(bin)).await;
                                    }
                                    Ok(Message::Ping(data)) => {
                                        let _ = write.send(Message::Pong(data)).await;
                                    }
                                    Ok(Message::Close(frame)) => {
                                        let _ = write.send(Message::Close(frame)).await;
                                        break;
                                    }
                                    _ => {}
                                }
                            }
                        }
                        Err(e) => {
                            eprintln!("WebSocket handshake error: {}", e);
                        }
                    }
                });
            }
            Err(e) => {
                eprintln!("Accept error: {}", e);
            }
        }
    }
}
