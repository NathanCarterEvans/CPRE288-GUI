use macroquad::prelude::*;
use crate::obstacle::ObstacleData;
use crate::scan::CybotScanData;

pub const CYBOT_RADIUS: f32 = 15_f32;
pub const SOUND_THICKNESS: f32 = 20.0;
pub const IR_THICKNESS: f32 = 5.0;
pub const SOUND_COLOR: Color = BLUE;
pub const IR_COLOR: Color = RED;

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

    pub fn draw_objs(&self, objs: &[ObstacleData]) {
        for obj in objs {
            let x = self.x + (obj.distance * (-1.0 * obj.angle_middle as f32).to_radians().cos());
            let y = self.y + (obj.distance * (-1.0 * obj.angle_middle as f32).to_radians().sin());
            obj.draw(x, y);
        }
    }
}
