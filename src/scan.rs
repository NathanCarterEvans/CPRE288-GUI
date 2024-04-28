use serde::{Deserialize, Serialize};
use macroquad::math::{f32, i32};
use macroquad::prelude::draw_line;
use crate::cybot::{IR_COLOR, IR_THICKNESS, SOUND_COLOR, SOUND_THICKNESS};

#[derive(Debug, Serialize, Deserialize)]
pub struct CybotScanData {
    pub angle: i32,
    pub distance: u32, // cm prob
}

impl CybotScanData {
    fn get_end_pt(&self, dist: f32, start_x: f32, start_y: f32) -> (f32, f32) {
        let deg = (self.angle) * -1;
        let x = start_x + (dist * (deg as f32).to_radians().cos());
        let y = start_y + (dist * (deg as f32).to_radians().sin());

        (x, y)
    }
    pub(crate) fn draw_scan_data(&self, start_x: f32, start_y: f32) {
        let (end_x, end_y) = self.get_end_pt(self.distance as f32, start_x, start_y);
        draw_line(start_x, start_y, end_x, end_y, SOUND_THICKNESS, SOUND_COLOR);
    }
}
