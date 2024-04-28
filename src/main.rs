pub mod cybot;

use std::{
    io::{Read, Write},
    net::TcpStream,
};
use std::net::SocketAddr;

use cybot::{Cybot, CybotScanData};
use macroquad::prelude::*;

#[macroquad::main("BasicShapes")]
async fn main() {
    let addr = SocketAddr::from(([127, 0, 0, 1], 288));
    let mut stream = TcpStream::connect(addr).expect("error connecting to host");
    let mut buf = [0_u8; 200];

    loop {
        clear_background(BLACK);
        let bytes_read = stream.read(&mut buf).expect("reading error");
        if bytes_read > 0 { println!("read {bytes_read:?} bytes"); }
        let msg = match serde_json::from_slice::<CybotScanData>(&buf[..bytes_read]) {
            Ok(data) => {
                let h = screen_height();
                let w = screen_width();
                // Draw robot
                let mut bot = Cybot::new(w / 2., h / 2.);
                // println!("{}", ser9e_json::to_string(&scan).unwrap());
                let s = format!("data: \n{data:?}");
                bot.data = Some(data);
                bot.draw_bot();
                s
            }
            Err(_) => {
                let s = String::from_utf8_lossy(&buf[..bytes_read]);

                s.to_string()
            }
        };
        if let Some(c) = get_char_pressed() {
            stream.write(&[c as u8]).expect("error writing");
        }

        if !msg.is_empty() { println!("{}", msg); }

        // draw_line(40.0, 40.0, 100.0, 200.0, 15.0, BLUE);
        // draw_rectangle(w/ 2.0 - 60.0, 100.0, 120.0, 60.0, GREEN);
        // draw_circle(w - 30.0, screen_height() - 30.0, 15.0, YELLOW);
        // draw_text("HELLO", 20.0, 20.0, 20.0, DARKGRAY);

        // draw_circle(w/2.,h/2., 14., YELLOW);
        next_frame().await
    }
}

// fn main() {
//     let mut stream = TcpStream::connect("192.168.1.1:288").expect("error connecting to host");
//     let mut buf = [0_u8; 200];
//     loop {
//     }
// }
