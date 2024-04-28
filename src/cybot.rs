use macroquad::prelude::*;
use serde::{Deserialize, Serialize};

pub const CYBOT_RADIUS: f32 = 15_f32;
pub const SOUND_THICKNESS: f32 = 20.0;
pub const IR_THICKNESS: f32 = 5.0;
pub const SOUND_COLOR: Color = BLUE;
pub const IR_COLOR: Color = RED;

#[derive(Debug, Serialize, Deserialize)]
pub struct CybotScanData {
    pub angle: i32, // 90 degrees is forward, 180 is left, 0 is right
    pub sound: f32, // cm
    pub ir: i32,
}
impl CybotScanData {
    fn get_end_pt(&self, dist: f32, start_x: f32, start_y: f32) -> (f32, f32) {
        let deg = (self.angle) * -1;
        let x = start_x + (dist * (deg as f32).to_radians().cos());
        let y = start_y + (dist * (deg as f32).to_radians().sin());

        (x, y)
    }
    fn draw_scan_data(&self, start_x: f32, start_y: f32) {
        let (end_x, end_y) = self.get_end_pt(self.sound, start_x, start_y);
        draw_line(start_x, start_y, end_x, end_y, SOUND_THICKNESS, SOUND_COLOR);
        let (end_x, end_y) = self.get_end_pt(self.ir as f32, start_x, start_y);
        draw_line(start_x, start_y, end_x, end_y, IR_THICKNESS, IR_COLOR);
    }
}
pub struct Cybot {
    x: f32,
    y: f32,
    pub data: Option<CybotScanData>,
}

impl Cybot {
    pub fn new(x: f32, y: f32) -> Self {
        Self { x, y, data: None }
    }
    pub fn draw_bot(&self) {
        draw_circle(self.x, self.y, CYBOT_RADIUS, YELLOW);
        if let Some(d) = &self.data {
            d.draw_scan_data(self.x, self.y);
        }
    }

    pub fn move_bot(&mut self, distance: f32, deg: f32) {}
}
