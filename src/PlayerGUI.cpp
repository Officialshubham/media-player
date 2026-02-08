#include "PlayerGUI.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <gst/video/videooverlay.h>
#include <gdk/gdk.h>

namespace fs = std::filesystem;

PlayerGUI::PlayerGUI() 
    : window(nullptr), video_area(nullptr), pipeline(nullptr), video_sink(nullptr),
      duration(0), timer_id(0), is_playing(false), is_fullscreen(false) {
    // Initialize GStreamer
    gst_init(nullptr, nullptr);
}

PlayerGUI::~PlayerGUI() {
    cleanup();
}

void PlayerGUI::cleanup() {
    std::cout << "Cleaning up resources..." << std::endl;
    
    // Remove timer
    if (timer_id) {
        g_source_remove(timer_id);
        timer_id = 0;
    }
    
    // Stop and cleanup GStreamer pipeline
    if (pipeline) {
        std::cout << "Stopping GStreamer pipeline..." << std::endl;
        gst_element_set_state(pipeline, GST_STATE_NULL);
        
        if (video_sink) {
            gst_object_unref(video_sink);
            video_sink = nullptr;
        }
        
        gst_object_unref(pipeline);
        pipeline = nullptr;
        std::cout << "GStreamer pipeline cleaned up" << std::endl;
    }
    
    is_playing = false;
}

void PlayerGUI::createUI() {
    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "vidc");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    
    // Create main vertical box
    GtkWidget* main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);
    
    // Create video area container (expands to fill space)
    video_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(video_container, TRUE);
    gtk_widget_set_hexpand(video_container, TRUE);
    gtk_box_pack_start(GTK_BOX(main_vbox), video_container, TRUE, TRUE, 0);
    
    // Create drawing area for video
    video_area = gtk_drawing_area_new();
    gtk_widget_set_vexpand(video_area, TRUE);
    gtk_widget_set_hexpand(video_area, TRUE);
    
    // Set black background for video area using CSS
    GtkCssProvider* css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, 
        "drawing-area { background-color: black; }", -1, NULL);
    GtkStyleContext* context = gtk_widget_get_style_context(video_area);
    gtk_style_context_add_provider(context, 
        GTK_STYLE_PROVIDER(css_provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider);
    
    gtk_widget_set_size_request(video_area, 640, 360);  // Minimum size
    gtk_box_pack_start(GTK_BOX(video_container), video_area, TRUE, TRUE, 0);
    
    // Create control panel at bottom
    GtkWidget* control_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(main_vbox), control_panel, FALSE, FALSE, 0);
    
    // File label
    file_label = gtk_label_new("No file loaded");
    gtk_label_set_xalign(GTK_LABEL(file_label), 0.0);
    gtk_widget_set_margin_start(file_label, 10);
    gtk_widget_set_margin_end(file_label, 10);
    gtk_box_pack_start(GTK_BOX(control_panel), file_label, FALSE, FALSE, 5);
    
    // Time label
    time_label = gtk_label_new("00:00 / 00:00");
    gtk_label_set_xalign(GTK_LABEL(time_label), 0.0);
    gtk_widget_set_margin_start(time_label, 10);
    gtk_widget_set_margin_end(time_label, 10);
    gtk_box_pack_start(GTK_BOX(control_panel), time_label, FALSE, FALSE, 5);
    
    // Seek scale
    seek_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(seek_scale), 0);
    gtk_widget_set_margin_start(seek_scale, 10);
    gtk_widget_set_margin_end(seek_scale, 10);
    gtk_box_pack_start(GTK_BOX(control_panel), seek_scale, FALSE, FALSE, 0);
    
    // Control buttons (horizontal box)
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(hbox, 10);
    gtk_widget_set_margin_end(hbox, 10);
    gtk_widget_set_margin_top(hbox, 10);
    gtk_widget_set_margin_bottom(hbox, 10);
    gtk_box_pack_start(GTK_BOX(control_panel), hbox, FALSE, FALSE, 0);
    
    // Open button
    open_button = gtk_button_new_with_label("📂 Open");
    gtk_widget_set_tooltip_text(open_button, "Open media file");
    gtk_box_pack_start(GTK_BOX(hbox), open_button, FALSE, FALSE, 0);
    
    // Play button
    play_button = gtk_button_new_with_label("▶ Play");
    gtk_widget_set_tooltip_text(play_button, "Play media");
    gtk_box_pack_start(GTK_BOX(hbox), play_button, FALSE, FALSE, 0);
    
    // Pause button
    pause_button = gtk_button_new_with_label("⏸ Pause");
    gtk_widget_set_tooltip_text(pause_button, "Pause media");
    gtk_box_pack_start(GTK_BOX(hbox), pause_button, FALSE, FALSE, 0);
    
    // Stop button
    stop_button = gtk_button_new_with_label("⏹ Stop");
    gtk_widget_set_tooltip_text(stop_button, "Stop media");
    gtk_box_pack_start(GTK_BOX(hbox), stop_button, FALSE, FALSE, 0);
    
    // Fullscreen button
    fullscreen_button = gtk_button_new_with_label("⛶ Fullscreen");
    gtk_widget_set_tooltip_text(fullscreen_button, "Toggle fullscreen (F or ESC)");
    gtk_box_pack_start(GTK_BOX(hbox), fullscreen_button, FALSE, FALSE, 0);
    
    // Volume label and scale
    GtkWidget* volume_label = gtk_label_new("Volume:");
    gtk_box_pack_start(GTK_BOX(hbox), volume_label, FALSE, FALSE, 0);
    
    volume_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(volume_scale), 50);
    gtk_widget_set_size_request(volume_scale, 100, -1);
    gtk_widget_set_tooltip_text(volume_scale, "Adjust volume");
    gtk_box_pack_start(GTK_BOX(hbox), volume_scale, TRUE, TRUE, 0);
    
    // Show all
    gtk_widget_show_all(window);
}

void PlayerGUI::setupCallbacks() {
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_close), this);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_window_key_press), this);
    g_signal_connect(open_button, "clicked", G_CALLBACK(on_open_clicked), this);
    g_signal_connect(play_button, "clicked", G_CALLBACK(on_play_clicked), this);
    g_signal_connect(pause_button, "clicked", G_CALLBACK(on_pause_clicked), this);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop_clicked), this);
    g_signal_connect(fullscreen_button, "clicked", G_CALLBACK(on_fullscreen_clicked), this);
    g_signal_connect(volume_scale, "value-changed", G_CALLBACK(on_volume_changed), this);
    g_signal_connect(seek_scale, "value-changed", G_CALLBACK(on_seek_changed), this);
    
    // Double-click on video area to toggle fullscreen
    g_signal_connect(video_area, "button-press-event", G_CALLBACK(+[](GtkWidget* widget, GdkEventButton* event, gpointer data) -> gboolean {
        if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
            PlayerGUI* player = static_cast<PlayerGUI*>(data);
            player->toggle_fullscreen();
            return TRUE;
        }
        return FALSE;
    }), this);
}

void PlayerGUI::show_error(const std::string& message) {
    GtkWidget* dialog = gtk_message_dialog_new(
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message.c_str());
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void PlayerGUI::run(int argc, char* argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);
    
    createUI();
    setupCallbacks();
    
    // Start timer for UI updates
    timer_id = g_timeout_add(100, update_ui, this);
    
    // Load file from command line if provided
    if (argc > 1) {
        load_file(argv[1]);
    }
    
    // Start GTK main loop
    gtk_main();
}

// Callback implementations
void PlayerGUI::on_open_clicked(GtkButton* button, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Open Media File",
        GTK_WINDOW(player->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL);
    
    // Set filters
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Media Files");
    gtk_file_filter_add_pattern(filter, "*.mp4");
    gtk_file_filter_add_pattern(filter, "*.mkv");
    gtk_file_filter_add_pattern(filter, "*.avi");
    gtk_file_filter_add_pattern(filter, "*.mov");
    gtk_file_filter_add_pattern(filter, "*.mp3");
    gtk_file_filter_add_pattern(filter, "*.wav");
    gtk_file_filter_add_pattern(filter, "*.webm");
    gtk_file_filter_add_pattern(filter, "*.flv");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    GtkFileFilter* all_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filter, "All Files");
    gtk_file_filter_add_pattern(all_filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        player->load_file(filename);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

void PlayerGUI::on_play_clicked(GtkButton* button, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    player->play();
}

void PlayerGUI::on_pause_clicked(GtkButton* button, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    player->pause();
}

void PlayerGUI::on_stop_clicked(GtkButton* button, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    player->stop();
}

void PlayerGUI::on_fullscreen_clicked(GtkButton* button, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    player->toggle_fullscreen();
}

void PlayerGUI::on_volume_changed(GtkRange* range, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    double volume = gtk_range_get_value(range) / 100.0;
    player->set_volume(volume);
}

void PlayerGUI::on_seek_changed(GtkRange* range, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    double percent = gtk_range_get_value(range);
    player->seek((percent / 100.0) * player->duration);
}

gboolean PlayerGUI::on_window_key_press(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    
    if (event->keyval == GDK_KEY_Escape) {
        if (player->is_fullscreen) {
            player->toggle_fullscreen();
            return TRUE;  // Event handled
        }
    } else if (event->keyval == GDK_KEY_f || event->keyval == GDK_KEY_F) {
        player->toggle_fullscreen();
        return TRUE;  // Event handled
    } else if (event->keyval == GDK_KEY_space) {
        if (player->is_playing) {
            player->pause();
        } else {
            player->play();
        }
        return TRUE;  // Event handled
    } else if (event->keyval == GDK_KEY_Left) {
        // Seek backward 5 seconds
        if (player->pipeline && player->duration > 0) {
            gint64 current = 0;
            if (gst_element_query_position(player->pipeline, GST_FORMAT_TIME, &current)) {
                gint64 new_position = current - 5 * GST_SECOND;
                if (new_position < 0) new_position = 0;
                player->seek(new_position);
            }
        }
        return TRUE;
    } else if (event->keyval == GDK_KEY_Right) {
        // Seek forward 5 seconds
        if (player->pipeline && player->duration > 0) {
            gint64 current = 0;
            if (gst_element_query_position(player->pipeline, GST_FORMAT_TIME, &current)) {
                gint64 new_position = current + 5 * GST_SECOND;
                if (new_position > player->duration) new_position = player->duration;
                player->seek(new_position);
            }
        }
        return TRUE;
    }
    
    return FALSE;  // Event not handled
}

gboolean PlayerGUI::on_window_close(GtkWidget* widget, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    
    // First cleanup resources
    player->cleanup();
    
    // Then quit GTK main loop
    gtk_main_quit();
    
    return FALSE;  // Allow default handler
}

gboolean PlayerGUI::bus_callback(GstBus* bus, GstMessage* msg, gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError* err;
            gchar* debug;
            gst_message_parse_error(msg, &err, &debug);
            
            std::string error_msg = std::string(err->message);
            if (debug) {
                error_msg += "\nDebug: " + std::string(debug);
            }
            
            player->show_error(error_msg);
            
            g_error_free(err);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_EOS:
            player->stop();
            break;
        case GST_MESSAGE_STATE_CHANGED: {
            GstState old_state, new_state, pending;
            gst_message_parse_state_changed(msg, &old_state, &new_state, &pending);
            
            // Update play/pause button state
            if (new_state == GST_STATE_PLAYING) {
                player->is_playing = true;
                gtk_button_set_label(GTK_BUTTON(player->play_button), "⏸ Pause");
            } else if (new_state == GST_STATE_PAUSED) {
                player->is_playing = false;
                gtk_button_set_label(GTK_BUTTON(player->play_button), "▶ Play");
            }
            break;
        }
    }
    
    return TRUE;
}

gboolean PlayerGUI::update_ui(gpointer data) {
    PlayerGUI* player = static_cast<PlayerGUI*>(data);
    
    if (player->pipeline) {
        // Update time display
        player->update_time_display();
        
        // Update seek slider if playing
        if (player->is_playing) {
            gint64 position = 0;
            if (gst_element_query_position(player->pipeline, GST_FORMAT_TIME, &position)) {
                if (player->duration > 0) {
                    double percent = (double)position / player->duration * 100.0;
                    // Block signal to prevent recursive call
                    g_signal_handlers_block_by_func(player->seek_scale, (gpointer)on_seek_changed, player);
                    gtk_range_set_value(GTK_RANGE(player->seek_scale), percent);
                    g_signal_handlers_unblock_by_func(player->seek_scale, (gpointer)on_seek_changed, player);
                }
            }
        }
    }
    
    return TRUE; // Continue timer
}

// Player control methods - Wayland specific
void PlayerGUI::load_file(const std::string& filename) {
    // Clean up previous pipeline
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
        video_sink = nullptr;
    }
    
    // First, check if file exists
    if (!fs::exists(filename)) {
        show_error("File not found: " + filename);
        return;
    }
    
    std::cout << "Loading file: " << filename << std::endl;
    
    // Ensure video area is realized
    if (!gtk_widget_get_realized(video_area)) {
        gtk_widget_realize(video_area);
    }
    
    // For Wayland, we need to use waylandsink or gtksink
    std::vector<std::string> pipeline_configs = {
        // Best option for Wayland: gtksink (embedded in GTK)
        "playbin uri=file://" + filename + " video-sink=gtksink name=videosink audio-sink=autoaudiosink",
        
        // Second option: waylandsink
        "playbin uri=file://" + filename + " video-sink=waylandsink name=videosink audio-sink=autoaudiosink",
        
        // Third option: autovideosink (will use waylandsink automatically on Wayland)
        "playbin uri=file://" + filename + " video-sink=autovideosink name=videosink audio-sink=autoaudiosink",
        
        // Fallback
        "filesrc location=\"" + filename + "\" ! decodebin ! videoconvert ! autovideosink name=videosink"
    };
    
    GError* error = nullptr;
    bool success = false;
    
    for (size_t i = 0; i < pipeline_configs.size() && !success; i++) {
        std::cout << "\nTrying pipeline " << (i+1) << ":\n" << pipeline_configs[i] << std::endl;
        
        pipeline = gst_parse_launch(pipeline_configs[i].c_str(), &error);
        
        if (error) {
            std::cerr << "❌ Error: " << error->message << std::endl;
            g_error_free(error);
            error = nullptr;
            if (pipeline) {
                gst_object_unref(pipeline);
                pipeline = nullptr;
            }
            continue;
        }
        
        if (pipeline) {
            // Get the video sink element
            video_sink = gst_bin_get_by_name(GST_BIN(pipeline), "videosink");
            
            // For Wayland, if we have a video overlay sink, set it up
            if (video_sink && GST_IS_VIDEO_OVERLAY(video_sink)) {
                // Get the GDK window
                GdkWindow* gdk_window = gtk_widget_get_window(video_area);
                if (gdk_window) {
                    // For Wayland, we need to use a different approach
                    // The surface handle is managed automatically by GTK/GDK
                    std::cout << "Setting up video overlay for Wayland" << std::endl;
                    
                    // Get video area dimensions
                    GtkAllocation allocation;
                    gtk_widget_get_allocation(video_area, &allocation);
                    
                    // Set video to fill the entire area
                    gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(video_sink), 
                                                           0, 0, 
                                                           allocation.width, 
                                                           allocation.height);
                    
                    // Expose the widget to GStreamer
                    gst_video_overlay_expose(GST_VIDEO_OVERLAY(video_sink));
                }
            }
            
            // First try to set to PAUSED state
            GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PAUSED);
            
            if (ret == GST_STATE_CHANGE_FAILURE) {
                std::cerr << "❌ Failed to go to PAUSED state" << std::endl;
                if (video_sink) {
                    gst_object_unref(video_sink);
                    video_sink = nullptr;
                }
                gst_object_unref(pipeline);
                pipeline = nullptr;
                continue;
            }
            
            // Wait for state change to complete
            ret = gst_element_get_state(pipeline, nullptr, nullptr, 2 * GST_SECOND);
            
            if (ret != GST_STATE_CHANGE_SUCCESS) {
                std::cerr << "❌ State change didn't complete" << std::endl;
                gst_element_set_state(pipeline, GST_STATE_NULL);
                if (video_sink) {
                    gst_object_unref(video_sink);
                    video_sink = nullptr;
                }
                gst_object_unref(pipeline);
                pipeline = nullptr;
                continue;
            }
            
            success = true;
            std::cout << "✅ Pipeline " << (i+1) << " successful!" << std::endl;
        }
    }
    
    if (!success || !pipeline) {
        // Show detailed error help
        std::string error_msg = "Failed to create pipeline for: " + filename;
        error_msg += "\n\nTroubleshooting steps:\n";
        error_msg += "1. Test with command line:\n";
        error_msg += "   gst-play-1.0 \"" + filename + "\"\n\n";
        error_msg += "2. Install Wayland-specific plugins:\n";
        error_msg += "   sudo pacman -S gst-plugins-good gst-plugins-bad gst-plugins-ugly\n";
        error_msg += "   sudo pacman -S gst-plugin-gtk\n";
        
        show_error(error_msg);
        return;
    }
    
    // Setup bus callback
    bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, bus_callback, this);
    gst_object_unref(bus);
    
    // Get duration
    duration = 0;
    if (!gst_element_query_duration(pipeline, GST_FORMAT_TIME, &duration)) {
        std::cerr << "Warning: Could not query duration" << std::endl;
    }
    
    current_file = filename;
    
    // Update UI
    std::string display_name = fs::path(filename).filename().string();
    gtk_label_set_text(GTK_LABEL(file_label), 
                      ("Loaded: " + display_name).c_str());
    
    // Reset time display
    std::string time_display = "00:00 / " + format_time(duration);
    gtk_label_set_text(GTK_LABEL(time_label), time_display.c_str());
    
    // Reset seek slider
    g_signal_handlers_block_by_func(seek_scale, (gpointer)on_seek_changed, this);
    gtk_range_set_value(GTK_RANGE(seek_scale), 0);
    g_signal_handlers_unblock_by_func(seek_scale, (gpointer)on_seek_changed, this);
    
    // AUTO-ENTER FULLSCREEN WHEN FILE IS LOADED
    if (!is_fullscreen) {
        toggle_fullscreen();
    }
    
    // Start playback automatically
    play();
    
    std::cout << "✅ Successfully loaded: " << filename << std::endl;
    std::cout << "Duration: " << (duration / GST_SECOND) << " seconds" << std::endl;
    std::cout << "Auto-entered fullscreen mode" << std::endl;
}

void PlayerGUI::play() {
    if (pipeline && !is_playing) {
        std::cout << "Attempting to start playback..." << std::endl;
        
        GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
        
        std::cout << "State change return: " << ret << std::endl;
        
        if (ret == GST_STATE_CHANGE_FAILURE) {
            // Try to get more error details
            GstBus* error_bus = gst_element_get_bus(pipeline);
            GstMessage* msg = gst_bus_timed_pop_filtered(error_bus, 0, 
                GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_WARNING));
            
            std::string error_msg = "Failed to start playback";
            
            if (msg) {
                GError* err;
                gchar* debug;
                if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                    gst_message_parse_error(msg, &err, &debug);
                    error_msg = std::string("Playback error: ") + err->message;
                    if (debug) {
                        error_msg += "\nDebug: " + std::string(debug);
                    }
                    g_error_free(err);
                    g_free(debug);
                }
                gst_message_unref(msg);
            }
            
            gst_object_unref(error_bus);
            
            // Try a different approach - set to READY first, then PLAYING
            std::cout << "Trying alternative playback method..." << std::endl;
            gst_element_set_state(pipeline, GST_STATE_READY);
            ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
            
            if (ret == GST_STATE_CHANGE_FAILURE) {
                show_error(error_msg + "\n\nTry:\n1. Install missing codecs\n2. Check video output");
                return;
            }
        }
        
        // Wait for state change to complete
        ret = gst_element_get_state(pipeline, nullptr, nullptr, 2 * GST_SECOND);
        if (ret == GST_STATE_CHANGE_SUCCESS) {
            is_playing = true;
            gtk_button_set_label(GTK_BUTTON(play_button), "⏸ Pause");
            std::cout << "✅ Playback started successfully!" << std::endl;
        } else {
            show_error("Playback started but state change didn't complete");
        }
    } else if (!pipeline) {
        show_error("No file loaded. Please open a media file first.");
    } else {
        // Already playing, toggle to pause
        pause();
    }
}

void PlayerGUI::pause() {
    if (pipeline && is_playing) {
        gst_element_set_state(pipeline, GST_STATE_PAUSED);
        is_playing = false;
        gtk_button_set_label(GTK_BUTTON(play_button), "▶ Play");
        std::cout << "Playback paused" << std::endl;
    }
}

void PlayerGUI::stop() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_READY);
        is_playing = false;
        gtk_button_set_label(GTK_BUTTON(play_button), "▶ Play");
        
        // Reset seek slider
        g_signal_handlers_block_by_func(seek_scale, (gpointer)on_seek_changed, this);
        gtk_range_set_value(GTK_RANGE(seek_scale), 0);
        g_signal_handlers_unblock_by_func(seek_scale, (gpointer)on_seek_changed, this);
        
        // Reset time display
        std::string time_display = "00:00 / " + format_time(duration);
        gtk_label_set_text(GTK_LABEL(time_label), time_display.c_str());
        
        std::cout << "Playback stopped" << std::endl;
    }
}

void PlayerGUI::toggle_fullscreen() {
    if (is_fullscreen) {
        gtk_window_unfullscreen(GTK_WINDOW(window));
        gtk_button_set_label(GTK_BUTTON(fullscreen_button), "Fullscreen");
        is_fullscreen = false;
        std::cout << "Exited fullscreen mode" << std::endl;
    } else {
        gtk_window_fullscreen(GTK_WINDOW(window));
        gtk_button_set_label(GTK_BUTTON(fullscreen_button), "Exit Fullscreen");
        is_fullscreen = true;
        std::cout << "Entered fullscreen mode" << std::endl;
    }
    
    // Update video overlay rectangle when resizing
    if (video_sink && GST_IS_VIDEO_OVERLAY(video_sink)) {
        // Get new video area dimensions
        GtkAllocation allocation;
        gtk_widget_get_allocation(video_area, &allocation);
        
        // Set video to fill the entire area
        gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(video_sink), 
                                               0, 0, 
                                               allocation.width, 
                                               allocation.height);
        
        // Expose again after resize
        gst_video_overlay_expose(GST_VIDEO_OVERLAY(video_sink));
    }
}

void PlayerGUI::set_volume(double volume) {
    if (pipeline) {
        // Try to set volume on audio sink
        GstElement* audio_sink = gst_bin_get_by_name(GST_BIN(pipeline), "audiosink");
        if (audio_sink) {
            g_object_set(audio_sink, "volume", volume, nullptr);
            gst_object_unref(audio_sink);
        } else {
            // Try setting on playbin directly
            g_object_set(pipeline, "volume", volume, nullptr);
        }
        std::cout << "Volume set to: " << (volume * 100) << "%" << std::endl;
    }
}

void PlayerGUI::seek(double position) {
    if (pipeline && duration > 0) {
        gint64 nanoseconds = static_cast<gint64>(position);
        bool success = gst_element_seek_simple(pipeline, GST_FORMAT_TIME,
                               GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                               nanoseconds);
        if (success) {
            std::cout << "Seeked to: " << (position / GST_SECOND) << " seconds" << std::endl;
            update_time_display();
        }
    }
}

void PlayerGUI::update_time_display() {
    if (pipeline) {
        gint64 position = 0;
        if (gst_element_query_position(pipeline, GST_FORMAT_TIME, &position)) {
            std::string time_str = format_time(position) + " / " + format_time(duration);
            gtk_label_set_text(GTK_LABEL(time_label), time_str.c_str());
        }
    }
}

std::string PlayerGUI::format_time(gint64 nanoseconds) {
    if (nanoseconds <= 0) return "00:00";
    
    gint64 total_seconds = nanoseconds / GST_SECOND;
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;
    
    char buffer[20];
    if (hours > 0) {
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
    } else {
        snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
    }
    return std::string(buffer);
}