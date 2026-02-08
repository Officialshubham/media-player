#ifndef PLAYER_GUI_HPP
#define PLAYER_GUI_HPP

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <string>
#include <chrono>

class PlayerGUI {
public:
    PlayerGUI();
    ~PlayerGUI();
    
    void run(int argc, char* argv[]);
    
private:
    // GTK widgets
    GtkWidget* window;
    GtkWidget* video_area;  // Widget for video display
    GtkWidget* play_button;
    GtkWidget* pause_button;
    GtkWidget* stop_button;
    GtkWidget* open_button;
    GtkWidget* volume_scale;
    GtkWidget* seek_scale;
    GtkWidget* time_label;
    GtkWidget* file_label;
    GtkWidget* fullscreen_button;
    GtkWidget* video_container;  // Container for video area
    
    // GStreamer
    GstElement* pipeline;
    GstElement* video_sink;
    GstBus* bus;
    
    // State
    std::string current_file;
    gint64 duration;
    guint timer_id;
    bool is_playing;
    bool is_fullscreen;
    
    // Private methods
    void createUI();
    void setupCallbacks();
    void show_error(const std::string& message);
    void create_video_sink();  // Create video sink widget
    
    // Static callbacks
    static void on_open_clicked(GtkButton* button, gpointer data);
    static void on_play_clicked(GtkButton* button, gpointer data);
    static void on_pause_clicked(GtkButton* button, gpointer data);
    static void on_stop_clicked(GtkButton* button, gpointer data);
    static void on_fullscreen_clicked(GtkButton* button, gpointer data);
    static void on_volume_changed(GtkRange* range, gpointer data);
    static void on_seek_changed(GtkRange* range, gpointer data);
    static gboolean on_window_close(GtkWidget* widget, gpointer data);
    static gboolean on_window_key_press(GtkWidget* widget, GdkEventKey* event, gpointer data);
    static gboolean on_video_area_realize(GtkWidget* widget, gpointer data);
    static gboolean bus_callback(GstBus* bus, GstMessage* msg, gpointer data);
    static gboolean update_ui(gpointer data);
    
    // Helper methods
    void load_file(const std::string& filename);
    void play();
    void pause();
    void stop();
    void set_volume(double volume);
    void seek(double position);
    void update_time_display();
    void toggle_fullscreen();
    void cleanup();
    std::string format_time(gint64 nanoseconds);
};

#endif // PLAYER_GUI_HPP