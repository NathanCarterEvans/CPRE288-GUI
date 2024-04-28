use macroquad::color::GREEN;
use macroquad::shapes::draw_circle_lines;

/// Draws the overlay centered on the x, y point
pub(crate) fn draw_overlay(x: f32, y: f32) {
    let overlays = (50..=2000).step_by(50);
    overlays.for_each(|r| {
        draw_circle_lines(x, y, r as f32, 1.0, GREEN);
    });
}
