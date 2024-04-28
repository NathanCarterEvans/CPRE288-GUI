use macroquad::color::GREEN;
use macroquad::math::{f32, i32};
use macroquad::prelude::draw_circle;
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct ObstacleData {
    pub distance: f32,
    pub angle_middle: i32, // middle of obstacle
    pub size: f32,         // arclen
}

impl ObstacleData {
    pub fn draw(&self, x: f32, y: f32) {
        draw_circle(x, y, self.size / 2_f32, GREEN);
    }
}
