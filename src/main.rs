pub mod cybot;
mod obstacle;
mod scan;
mod overlay;

use std::{
    io::{Read, Write},
    net::{SocketAddr, TcpStream},
};

use cybot::Cybot;
use macroquad::prelude::*;

use obstacle::ObstacleData;
use scan::CybotScanData;

#[macroquad::main("BasicShapes")]
async fn main() {
    let mut objs = vec![];
    let addr = SocketAddr::from(([127, 0, 0, 1], 288));
    let mut stream = TcpStream::connect(addr).expect("error connecting to host");
    println!("connected");
    let mut buf = [0_u8; 200];

    let mut bot = Cybot::new(0_f32,0_f32);
    loop {
        let h = screen_height();
        let w = screen_width();
        bot.x = w/2_f32;
        bot.y = h * 0.8;
        clear_background(BLACK);
        if let Some(c) = get_char_pressed() {
            stream.write(&[c as u8]).expect("error writing");
        }

        let read_res = stream.read(&mut buf);
        if let Ok(bytes_read) = read_res {
            if let Ok(d) = serde_json::from_slice::<ObstacleData>(&buf[..bytes_read]) {
                println!("{:?}", d);
                objs.push(d);
                println!("{:?}", objs);
            }

            println!("read {bytes_read:?} bytes");
            let msg = update_bot_with_msg(&mut bot, &buf[..bytes_read]);
            println!("{}", msg);
        }

        bot.draw_bot();
        bot.draw_objs(&objs);
        next_frame().await
    }
}

fn update_bot_with_msg(bot: &mut Cybot, read_data: &[u8]) -> String {
    match serde_json::from_slice::<CybotScanData>(read_data) {
        Ok(data) => {
            let s = format!("data: \n{data:?}");
            bot.data = Some(data);
            s
        }
        Err(_) => {
            let s = String::from_utf8_lossy(read_data);
            s.to_string()
        }
    }
}
