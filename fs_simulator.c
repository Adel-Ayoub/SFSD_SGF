#include <gtk/gtk.h>
#include "stdlib.h"
#include "stdio.h"
// Structure definitions for GUI elements

typedef struct {
    GtkWidget *dialog;
    GtkWidget *name_entry;
    GtkWidget *records_spin;
    GtkWidget *global_org_combo;
    GtkWidget *internal_org_combo;
} CreateFileDialog;

typedef struct {
    GtkWidget *dialog;
    GtkWidget *blocks_spin;
    GtkWidget *block_size_spin;
} InitDiskDialog;

typedef struct {
    GtkWidget *dialog;
    GtkWidget *file_combo;
    GtkWidget *id_entry;
} SearchRecordDialog;
// Global widgets
GtkWidget *window;
GtkWidget *file_tree_view;
GtkWidget *disk_view_drawing_area;
GtkWidget *metadata_view;
GtkWidget *status_bar;
GtkWidget *toolbar;

// Drawing area size
#define BLOCK_SIZE 50
#define BLOCK_SPACING 5
#define BLOCKS_PER_ROW 10

// Create the file tree view
GtkWidget *create_file_tree_view() {
    // Create a scrolled window to contain the tree view
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    // Create tree store
    GtkTreeStore *store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);  // Name, Type

    // Create tree view
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    // Create columns
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Name",
                                                                         renderer,
                                                                         "text", 0,
                                                                         NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    column = gtk_tree_view_column_new_with_attributes("Type",
                                                      renderer,
                                                      "text", 1,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    // Add some example items (replace with actual file system data)
    GtkTreeIter iter;
    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter, 0, "File1", 1, "Contiguous", -1);

    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter, 0, "File2", 1, "Chained", -1);

    gtk_container_add(GTK_CONTAINER(scroll), tree);

    return scroll;
}

// Update file tree view with actual file system data
void update_file_tree_view() {
    GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(
            GTK_TREE_VIEW(gtk_bin_get_child(GTK_BIN(file_tree_view)))));

    // Clear existing items
    gtk_tree_store_clear(store);

    // Add items based on actual file system state
    // This is where you'll add your actual file system data
    GtkTreeIter iter;
    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter, 0, "Example File", 1, "Contiguous", -1);
}

// Forward declarations
void show_init_disk_dialog(GtkWidget *parent);
void show_create_file_dialog(GtkWidget *parent);
void show_search_record_dialog(GtkWidget *parent);
void show_insert_record_dialog(GtkWidget *parent);
void show_delete_record_dialog(GtkWidget *parent);
void show_rename_file_dialog(GtkWidget *parent);
gboolean draw_disk_view(GtkWidget *widget, cairo_t *cr, gpointer data);

// Toolbar button callbacks
void on_init_disk_clicked(GtkToolButton *button, gpointer user_data) {
    show_init_disk_dialog(GTK_WIDGET(window));
}

void on_create_file_clicked(GtkToolButton *button, gpointer user_data) {
    show_create_file_dialog(GTK_WIDGET(window));
}

void on_search_record_clicked(GtkToolButton *button, gpointer user_data) {
    show_search_record_dialog(GTK_WIDGET(window));
}

void on_insert_record_clicked(GtkToolButton *button, gpointer user_data) {
    show_insert_record_dialog(GTK_WIDGET(window));
}

void on_delete_record_clicked(GtkToolButton *button, gpointer user_data) {
    show_delete_record_dialog(GTK_WIDGET(window));
}

void on_rename_file_clicked(GtkToolButton *button, gpointer user_data) {
    show_rename_file_dialog(GTK_WIDGET(window));
}

void on_compact_disk_clicked(GtkToolButton *button, gpointer user_data) {
    // Placeholder for disk compaction logic
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "Disk compacted");
    gtk_widget_queue_draw(disk_view_drawing_area);
}

void on_clear_disk_clicked(GtkToolButton *button, gpointer user_data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_YES_NO,
                                               "Are you sure you want to clear the disk?");

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_YES) {
        // Placeholder for disk clearing logic
        gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "Disk cleared");
        gtk_widget_queue_draw(disk_view_drawing_area);
    }
    gtk_widget_destroy(dialog);
}

// Dialog implementations continued from previous code...

void show_search_record_dialog(GtkWidget *parent) {
    SearchRecordDialog dialog;

    dialog.dialog = gtk_dialog_new_with_buttons("Search Record",
                                                GTK_WINDOW(parent),
                                                GTK_DIALOG_MODAL,
                                                "Cancel",
                                                GTK_RESPONSE_CANCEL,
                                                "Search",
                                                GTK_RESPONSE_ACCEPT,
                                                NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog.dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    // File selection
    GtkWidget *file_label = gtk_label_new("Select file:");
    dialog.file_combo = gtk_combo_box_text_new();
    // Populate with existing files (placeholder)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog.file_combo), "File1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog.file_combo), "File2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(dialog.file_combo), 0);

    // Record ID
    GtkWidget *id_label = gtk_label_new("Record ID:");
    dialog.id_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), file_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.file_combo, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.id_entry, 1, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog.dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog.dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        // Placeholder for search logic
        gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "Record searched");
    }

    gtk_widget_destroy(dialog.dialog);
}

void show_insert_record_dialog(GtkWidget *parent) {
    // Similar to search dialog but with fields for record data
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "Insert Record functionality to be implemented");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_delete_record_dialog(GtkWidget *parent) {
    // Similar to search dialog but with delete confirmation
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "Delete Record functionality to be implemented");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_rename_file_dialog(GtkWidget *parent) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Rename File",
                                                    GTK_WINDOW(parent),
                                                    GTK_DIALOG_MODAL,
                                                    "Cancel",
                                                    GTK_RESPONSE_CANCEL,
                                                    "Rename",
                                                    GTK_RESPONSE_ACCEPT,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    // File selection combo box
    GtkWidget *file_label = gtk_label_new("Select file:");
    GtkWidget *file_combo = gtk_combo_box_text_new();
    // Populate with existing files (placeholder)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(file_combo), "File1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(file_combo), "File2");

    // New name entry
    GtkWidget *name_label = gtk_label_new("New name:");
    GtkWidget *name_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), file_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), file_combo, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        // Placeholder for rename logic
        gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "File renamed");
    }

    gtk_widget_destroy(dialog);
}

// Create the main toolbar
GtkWidget *create_toolbar() {
    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);

    // Initialize Disk
    GtkToolItem *init_button = gtk_tool_button_new(NULL, "Init Disk");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), init_button, -1);
    g_signal_connect(init_button, "clicked", G_CALLBACK(on_init_disk_clicked), NULL);

    // Create File
    GtkToolItem *create_button = gtk_tool_button_new(NULL, "Create File");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), create_button, -1);
    g_signal_connect(create_button, "clicked", G_CALLBACK(on_create_file_clicked), NULL);

    // Search Record
    GtkToolItem *search_button = gtk_tool_button_new(NULL, "Search");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), search_button, -1);
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_record_clicked), NULL);

    // Insert Record
    GtkToolItem *insert_button = gtk_tool_button_new(NULL, "Insert");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), insert_button, -1);
    g_signal_connect(insert_button, "clicked", G_CALLBACK(on_insert_record_clicked), NULL);

    // Delete Record
    GtkToolItem *delete_button = gtk_tool_button_new(NULL, "Delete");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), delete_button, -1);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_record_clicked), NULL);

    // Rename File
    GtkToolItem *rename_button = gtk_tool_button_new(NULL, "Rename");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), rename_button, -1);
    g_signal_connect(rename_button, "clicked", G_CALLBACK(on_rename_file_clicked), NULL);

    // Compact Disk
    GtkToolItem *compact_button = gtk_tool_button_new(NULL, "Compact");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), compact_button, -1);
    g_signal_connect(compact_button, "clicked", G_CALLBACK(on_compact_disk_clicked), NULL);

    // Clear Disk
    GtkToolItem *clear_button = gtk_tool_button_new(NULL, "Clear");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), clear_button, -1);
    g_signal_connect(clear_button, "clicked", G_CALLBACK(on_clear_disk_clicked), NULL);

    return toolbar;
}

// Create metadata view
GtkWidget *create_metadata_view() {
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);

    // Placeholder text
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, "Metadata will be shown here", -1);

    return scroll;
}




// Drawing function for disk visualization
gboolean draw_disk_view(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Example drawing - replace with actual disk state
    int total_blocks = 20; // This should come from your disk management logic
    int blocks_per_row = BLOCKS_PER_ROW;
    int current_row = 0;
    int current_col = 0;

    for (int i = 0; i < total_blocks; i++) {
        // Calculate position
        int x = BLOCK_SPACING + current_col * (BLOCK_SIZE + BLOCK_SPACING);
        int y = BLOCK_SPACING + current_row * (BLOCK_SIZE + BLOCK_SPACING);

        // Example: Alternate between free and occupied blocks
        if (i % 2 == 0) {
            // Free block (green)
            cairo_set_source_rgb(cr, 0.2, 0.8, 0.2);
        } else {
            // Occupied block (red)
            cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
        }

        // Draw block
        cairo_rectangle(cr, x, y, BLOCK_SIZE, BLOCK_SIZE);
        cairo_fill(cr);

        // Draw block border
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_rectangle(cr, x, y, BLOCK_SIZE, BLOCK_SIZE);
        cairo_stroke(cr);

        // Example: Add text for occupied blocks
        if (i % 2 != 0) {
            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size(cr, 10);

            char block_text[32];
            snprintf(block_text, sizeof(block_text), "File %d\n(3 rec)", i);

            cairo_move_to(cr, x + 5, y + 20);
            cairo_show_text(cr, block_text);
        }

        // Update position
        current_col++;
        if (current_col >= blocks_per_row) {
            current_col = 0;
            current_row++;
        }
    }

    return FALSE;
}

// Function to create and show the initialize disk dialog
void show_init_disk_dialog(GtkWidget *parent) {
    InitDiskDialog dialog;

    dialog.dialog = gtk_dialog_new_with_buttons("Initialize Disk",
                                                GTK_WINDOW(parent),
                                                GTK_DIALOG_MODAL,
                                                "Cancel",
                                                GTK_RESPONSE_CANCEL,
                                                "Initialize",
                                                GTK_RESPONSE_ACCEPT,
                                                NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog.dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    // Number of blocks
    GtkWidget *blocks_label = gtk_label_new("Number of blocks:");
    dialog.blocks_spin = gtk_spin_button_new_with_range(1, 1000, 1);
    gtk_grid_attach(GTK_GRID(grid), blocks_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.blocks_spin, 1, 0, 1, 1);

    // Block size
    GtkWidget *size_label = gtk_label_new("Block size:");
    dialog.block_size_spin = gtk_spin_button_new_with_range(64, 4096, 64);
    gtk_grid_attach(GTK_GRID(grid), size_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.block_size_spin, 1, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog.dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog.dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        // Placeholder for disk initialization logic
        gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "Disk initialized");
    }

    gtk_widget_destroy(dialog.dialog);
}

// Function to create and show the create file dialog
void show_create_file_dialog(GtkWidget *parent) {
    CreateFileDialog dialog;

    dialog.dialog = gtk_dialog_new_with_buttons("Create File",
                                                GTK_WINDOW(parent),
                                                GTK_DIALOG_MODAL,
                                                "Cancel",
                                                GTK_RESPONSE_CANCEL,
                                                "Create",
                                                GTK_RESPONSE_ACCEPT,
                                                NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog.dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    // File name
    GtkWidget *name_label = gtk_label_new("File name:");
    dialog.name_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.name_entry, 1, 0, 1, 1);

    // Number of records
    GtkWidget *records_label = gtk_label_new("Number of records:");
    dialog.records_spin = gtk_spin_button_new_with_range(1, 1000, 1);
    gtk_grid_attach(GTK_GRID(grid), records_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.records_spin, 1, 1, 1, 1);

    // Global organization
    GtkWidget *global_label = gtk_label_new("Global organization:");
    dialog.global_org_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog.global_org_combo), "Contiguous");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog.global_org_combo), "Chained");
    gtk_combo_box_set_active(GTK_COMBO_BOX(dialog.global_org_combo), 0);
    gtk_grid_attach(GTK_GRID(grid), global_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.global_org_combo, 1, 2, 1, 1);

    // Internal organization
    GtkWidget *internal_label = gtk_label_new("Internal organization:");
    dialog.internal_org_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog.internal_org_combo), "Sorted");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog.internal_org_combo), "Unsorted");
    gtk_combo_box_set_active(GTK_COMBO_BOX(dialog.internal_org_combo), 0);
    gtk_grid_attach(GTK_GRID(grid), internal_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.internal_org_combo, 1, 3, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog.dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog.dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        // Placeholder for file creation logic
        gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "File created");
    }

    gtk_widget_destroy(dialog.dialog);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "File System Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create main vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Add toolbar
    toolbar = create_toolbar();
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    // Create horizontal box for main content
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    // Add file tree view (left panel)
    file_tree_view = create_file_tree_view();
    gtk_widget_set_size_request(file_tree_view, 250, -1);
    gtk_box_pack_start(GTK_BOX(hbox), file_tree_view, FALSE, FALSE, 0);

    // Create right panel for disk view and metadata
    GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), right_vbox, TRUE, TRUE, 0);

    // Add disk view (only shown when selecting an operation)
    disk_view_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(disk_view_drawing_area, 400, 300);
    gtk_widget_set_no_show_all(disk_view_drawing_area, TRUE);  // Hidden by default
    gtk_box_pack_start(GTK_BOX(right_vbox), disk_view_drawing_area, TRUE, TRUE, 0);
    g_signal_connect(disk_view_drawing_area, "draw", G_CALLBACK(draw_disk_view), NULL);

    // Add metadata view
    metadata_view = create_metadata_view();
    gtk_widget_set_size_request(metadata_view, -1, 200);
    gtk_box_pack_start(GTK_BOX(right_vbox), metadata_view, FALSE, FALSE, 0);

    // Add status bar
    status_bar = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(vbox), status_bar, FALSE, FALSE, 0);
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "Ready");

    // Show all widgets except disk view
    gtk_widget_show_all(window);
    gtk_widget_hide(disk_view_drawing_area);

    // Start the GTK main loop
    gtk_main();

    return 0;
}

// Function to show disk view when needed
void show_disk_view(gboolean show) {
    if (show) {
        gtk_widget_show(disk_view_drawing_area);
    } else {
        gtk_widget_hide(disk_view_drawing_area);
    }
}

// Function to update metadata view with file information
void update_metadata_view(const char *metadata_text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_bin_get_child(GTK_BIN(metadata_view))));
    gtk_text_buffer_set_text(buffer, metadata_text, -1);
}

// Function to refresh disk view
void refresh_disk_view() {
    gtk_widget_queue_draw(disk_view_drawing_area);
}

// Function to show error dialog
void show_error_dialog(GtkWidget *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Function to show success dialog
void show_success_dialog(GtkWidget *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Create the file tree view
