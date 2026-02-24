#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

static lv_color_t draw_buf[SCREEN_WIDTH * 20];
static lv_display_t *display;

TFT_eSPI tft = TFT_eSPI();

// Panel containers 
lv_obj_t *temp_panel;
lv_obj_t *pressure_panel;
lv_obj_t *chart_panel;
lv_obj_t *temp_arc;
lv_obj_t *pressure_arc;
lv_obj_t *temp_value_label;
lv_obj_t *pressure_value_label;
lv_obj_t *chart;
lv_chart_series_t *temp_series;
// lv_chart_series_t *pressure_series;
lv_obj_t *pressure_chart;
lv_chart_series_t *pressure_series;



void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();
    lv_display_flush_ready(disp);
}

static uint32_t my_tick(void) {
    return millis();
}


void create_dashboard() {
    // Root screen 
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    // Title label 
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "BMP180");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x333333), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);


    // Chart panel 
    //  Left graph panel (Temperature) ─
    lv_obj_t *temp_chart_panel = lv_obj_create(scr);
    lv_obj_set_size(temp_chart_panel, 159, 120);
    lv_obj_align(temp_chart_panel, LV_ALIGN_TOP_LEFT, 0, 35);
    lv_obj_set_style_bg_color(temp_chart_panel, lv_color_white(), 0);
    lv_obj_set_style_border_width(temp_chart_panel, 1, 0);
    lv_obj_set_style_border_color(temp_chart_panel, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_radius(temp_chart_panel, 6, 0);
    lv_obj_set_style_pad_all(temp_chart_panel, 4, 0);
    lv_obj_set_scroll_dir(temp_chart_panel, LV_DIR_NONE);

    // T label
    lv_obj_t *t_label = lv_label_create(temp_chart_panel);
    lv_label_set_text(t_label, "T");
    lv_obj_set_style_text_font(t_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(t_label, lv_color_hex(0x4CAF50), 0);
    lv_obj_align(t_label, LV_ALIGN_TOP_LEFT, 2, 0);

    // Temp chart
    chart = lv_chart_create(temp_chart_panel);
    lv_obj_set_size(chart, 140, 90);
    lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, 20);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 22, 28);
    lv_chart_set_div_line_count(chart, 4, 5);
    lv_obj_set_style_bg_color(chart, lv_color_white(), 0);
    lv_obj_set_style_border_width(chart, 0, 0);
    lv_obj_set_style_line_color(chart, lv_color_hex(0xEEEEEE), LV_PART_MAIN);
    lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);

    temp_series = lv_chart_add_series(chart, lv_color_hex(0x4CAF50),
                                      LV_CHART_AXIS_PRIMARY_Y);

    // Pre-fill temp data
    for(int i = 0; i < 20; i++) {
        lv_chart_set_next_value(chart, temp_series, 25);
    }

    //  Right graph panel (Pressure) 
    lv_obj_t *pressure_chart_panel = lv_obj_create(scr);
    lv_obj_set_size(pressure_chart_panel, 159, 120);
    lv_obj_align(pressure_chart_panel, LV_ALIGN_TOP_RIGHT, 0, 35);
    lv_obj_set_style_bg_color(pressure_chart_panel, lv_color_white(), 0);
    lv_obj_set_style_border_width(pressure_chart_panel, 1, 0);
    lv_obj_set_style_border_color(pressure_chart_panel, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_radius(pressure_chart_panel, 6, 0);
    lv_obj_set_style_pad_all(pressure_chart_panel, 4, 0);
    lv_obj_set_scroll_dir(pressure_chart_panel, LV_DIR_NONE);

    // P label
    lv_obj_t *p_label = lv_label_create(pressure_chart_panel);
    lv_label_set_text(p_label, "P");
    lv_obj_set_style_text_font(p_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(p_label, lv_color_hex(0x2196F3), 0);
    lv_obj_align(p_label, LV_ALIGN_TOP_LEFT, 2, 0);

    // Pressure chart
    pressure_chart = lv_chart_create(pressure_chart_panel);
    lv_obj_set_size(pressure_chart, 140, 90);
    lv_obj_align(pressure_chart, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_chart_set_type(pressure_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(pressure_chart, 20);
    lv_chart_set_range(pressure_chart, LV_CHART_AXIS_PRIMARY_Y, 806, 816);
    lv_chart_set_div_line_count(pressure_chart, 4, 5);
    lv_obj_set_style_bg_color(pressure_chart, lv_color_white(), 0);
    lv_obj_set_style_border_width(pressure_chart, 0, 0);
    lv_obj_set_style_line_color(pressure_chart, lv_color_hex(0xEEEEEE), LV_PART_MAIN);
    lv_obj_set_style_size(pressure_chart, 0, 0, LV_PART_INDICATOR);

    pressure_series = lv_chart_add_series(pressure_chart, lv_color_hex(0x2196F3),
                                          LV_CHART_AXIS_PRIMARY_Y);

    // Pre-fill pressure data
    // for(int i = 0; i < 20; i++) {
    //     lv_chart_set_next_value(pressure_chart, pressure_series, 811);
    // }
                 
    // Pressure graph scale labels
    lv_obj_t *p_max = lv_label_create(pressure_chart_panel);
    lv_label_set_text(p_max, "816");
    lv_obj_set_style_text_font(p_max, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(p_max, lv_color_hex(0x999999), 0);
    lv_obj_align(p_max, LV_ALIGN_TOP_RIGHT, 2, 2);

    lv_obj_t *p_mid = lv_label_create(pressure_chart_panel);
    lv_label_set_text(p_mid, "811");
    lv_obj_set_style_text_font(p_mid, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(p_mid, lv_color_hex(0x999999), 0);
    lv_obj_align(p_mid, LV_ALIGN_RIGHT_MID, 2, 0);

    lv_obj_t *p_min = lv_label_create(pressure_chart_panel);
    lv_label_set_text(p_min, "806");
    lv_obj_set_style_text_font(p_min, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(p_min, lv_color_hex(0x999999), 0);
    lv_obj_align(p_min, LV_ALIGN_BOTTOM_RIGHT, 2, -8);
    // Pre-fill with demo data so chart is not empty
    // for(int i = 0; i < 20; i++) {
    //     lv_chart_set_next_value(chart, temp_series, 20 + (i % 5));
    //     lv_chart_set_next_value(chart, pressure_series, 811 + (i % 8));
    // }

    // Temp graph scale labels
    lv_obj_t *t_max = lv_label_create(temp_chart_panel);
    lv_label_set_text(t_max, "28");
    lv_obj_set_style_text_font(t_max, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(t_max, lv_color_hex(0x999999), 0);
    lv_obj_align(t_max, LV_ALIGN_TOP_RIGHT, 2, 2);

    lv_obj_t *t_mid = lv_label_create(temp_chart_panel);
    lv_label_set_text(t_mid, "25");
    lv_obj_set_style_text_font(t_mid, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(t_mid, lv_color_hex(0x999999), 0);
    lv_obj_align(t_mid, LV_ALIGN_RIGHT_MID, 2, 0);

    lv_obj_t *t_min = lv_label_create(temp_chart_panel);
    lv_label_set_text(t_min, "22");
    lv_obj_set_style_text_font(t_min, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(t_min, lv_color_hex(0x999999), 0);
    lv_obj_align(t_min, LV_ALIGN_BOTTOM_RIGHT, 2, -8);

    // Temp panel 
    temp_panel = lv_obj_create(scr);
    lv_obj_set_size(temp_panel, 158, 100);
    lv_obj_align(temp_panel, LV_ALIGN_BOTTOM_LEFT, 1, -1);
    lv_obj_set_style_bg_color(temp_panel, lv_color_white(), 0);
    lv_obj_set_style_border_width(temp_panel, 1, 0);
    lv_obj_set_style_border_color(temp_panel, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_radius(temp_panel, 6, 0);
    lv_obj_set_style_pad_all(temp_panel, 4, 0);

    lv_obj_t *temp_title = lv_label_create(temp_panel);
    lv_label_set_text(temp_title, "Temperature");
    lv_obj_set_style_text_font(temp_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(temp_title, lv_color_hex(0x333333), 0);
    lv_obj_align(temp_title, LV_ALIGN_TOP_LEFT, 4, 2);

    // Temp arc gauge
    temp_arc = lv_arc_create(temp_panel);
    lv_obj_set_size(temp_arc, 70, 70);
    lv_arc_set_rotation(temp_arc, 135);
    lv_arc_set_bg_angles(temp_arc, 0, 270);
    lv_arc_set_range(temp_arc, -10, 50);
    lv_arc_set_value(temp_arc, 25);
    lv_obj_remove_style(temp_arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_color(temp_arc, lv_color_hex(0x4CAF50), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(temp_arc, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
    lv_obj_set_style_arc_width(temp_arc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(temp_arc, 8, LV_PART_MAIN);
    lv_obj_align(temp_arc, LV_ALIGN_CENTER, 10, 8);

    // Temp value label (inside arc)
    temp_value_label = lv_label_create(temp_panel);
    lv_label_set_text(temp_value_label, "25°C");
    lv_obj_set_style_text_font(temp_value_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(temp_value_label, lv_color_hex(0x333333), 0);
    lv_obj_align_to(temp_value_label, temp_arc, LV_ALIGN_CENTER, -8, 0);

    // Temp scale labels
    lv_obj_t *temp_min = lv_label_create(temp_panel);
    lv_label_set_text(temp_min, "-10");
    lv_obj_set_style_text_font(temp_min, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(temp_min, lv_color_hex(0x999999), 0);
    lv_obj_align_to(temp_min, temp_arc, LV_ALIGN_OUT_BOTTOM_LEFT, 0, -8);

    lv_obj_t *temp_max = lv_label_create(temp_panel);
    lv_label_set_text(temp_max, "50");
    lv_obj_set_style_text_font(temp_max, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(temp_max, lv_color_hex(0x999999), 0);
    lv_obj_align_to(temp_max, temp_arc, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -8);

    // pressure panel 
    pressure_panel = lv_obj_create(scr);
    lv_obj_set_size(pressure_panel, 158, 100);
    lv_obj_align(pressure_panel, LV_ALIGN_BOTTOM_RIGHT, -1, -1);
    lv_obj_set_style_bg_color(pressure_panel, lv_color_white(), 0);
    lv_obj_set_style_border_width(pressure_panel, 1, 0);
    lv_obj_set_style_border_color(pressure_panel, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_radius(pressure_panel, 6, 0);
    lv_obj_set_style_pad_all(pressure_panel, 4, 0);

    lv_obj_t *hum_title = lv_label_create(pressure_panel);
    lv_label_set_text(hum_title, "Pressure");
    lv_obj_set_style_text_font(hum_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hum_title, lv_color_hex(0x333333), 0);
    lv_obj_align(hum_title, LV_ALIGN_TOP_LEFT, 4, 2);

    // pressure arc gauge
    pressure_arc = lv_arc_create(pressure_panel);
    lv_obj_set_size(pressure_arc, 70, 70);
    lv_arc_set_rotation(pressure_arc, 135);
    lv_arc_set_bg_angles(pressure_arc, 0, 270);
    lv_arc_set_range(pressure_arc, 750, 850);
    lv_arc_set_value(pressure_arc, 811);
    lv_obj_remove_style(pressure_arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_color(pressure_arc, lv_color_hex(0x2196F3), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(pressure_arc, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
    lv_obj_set_style_arc_width(pressure_arc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(pressure_arc, 8, LV_PART_MAIN);
    lv_obj_align(pressure_arc, LV_ALIGN_CENTER, 10, 8);

    // pressure value label (inside arc)
    pressure_value_label = lv_label_create(pressure_panel);
    lv_label_set_text(pressure_value_label, "60%");
    lv_obj_set_style_text_font(pressure_value_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(pressure_value_label, lv_color_hex(0x333333), 0);
    lv_obj_align_to(pressure_value_label, pressure_arc, LV_ALIGN_CENTER, -8, 0);

    // Pressure scale labels
    lv_obj_t *pres_min = lv_label_create(pressure_panel);
    lv_label_set_text(pres_min, "780");
    lv_obj_set_style_text_font(pres_min, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(pres_min, lv_color_hex(0x999999), 0);
    lv_obj_align_to(pres_min, pressure_arc, LV_ALIGN_OUT_BOTTOM_LEFT, 0, -8);

    lv_obj_t *pres_max = lv_label_create(pressure_panel);
    lv_label_set_text(pres_max, "840");
    lv_obj_set_style_text_font(pres_max, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(pres_max, lv_color_hex(0x999999), 0);
    lv_obj_align_to(pres_max, pressure_arc, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -8);

    
}

void setup() {
    Serial.begin(115200);
    Wire.begin(27, 22);
    bmp.begin();

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    lv_init();
    lv_tick_set_cb(my_tick);

    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(display, my_disp_flush);
    lv_display_set_buffers(display, draw_buf, NULL,
                           sizeof(draw_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    create_dashboard();
}
void loop() {
    lv_timer_handler();

    static uint32_t last_update = 0;
    if (millis() - last_update > 2000) {
        last_update = millis();

        float real_temp = bmp.readTemperature();
        float real_pressure = bmp.readPressure() / 100.0;

        if (real_temp > -40 && real_temp < 80) {
            // Update chart
            lv_chart_set_next_value(chart, temp_series, (int)real_temp);
            lv_chart_set_next_value(pressure_chart, pressure_series, (int)real_pressure);
            lv_chart_refresh(chart);
            lv_chart_refresh(pressure_chart);

            // Update arc gauges
            lv_arc_set_value(temp_arc, (int)real_temp);
            lv_arc_set_value(pressure_arc, (int)real_pressure);

            // Update value labels
            char buf[16];
            snprintf(buf, sizeof(buf), "%.1f°C", real_temp);
            lv_label_set_text(temp_value_label, buf);
            snprintf(buf, sizeof(buf), "%.0fhPa", real_pressure);
            lv_label_set_text(pressure_value_label, buf);

            // Temp alert colors
            if (real_temp > 35.0 || real_temp < 10.0) {
                lv_obj_set_style_arc_color(temp_arc, lv_color_hex(0xE53935), LV_PART_INDICATOR);
            } else {
                lv_obj_set_style_arc_color(temp_arc, lv_color_hex(0x4CAF50), LV_PART_INDICATOR);
            }

            // Pressure alert colors (normal range 980-1020 hPa)
            if (real_pressure < 790 || real_pressure > 830) {
                lv_obj_set_style_arc_color(pressure_arc, lv_color_hex(0xE53935), LV_PART_INDICATOR);
            } else {
                lv_obj_set_style_arc_color(pressure_arc, lv_color_hex(0x2196F3), LV_PART_INDICATOR);
            }
        }
    }

    delay(5);
}
