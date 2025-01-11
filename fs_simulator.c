#include <gtk/gtk.h>
#include "stdlib.h"
#include "stdio.h"
#include "TNOF.h"
#include "gen.h"
// Structure definitions for GUI elements
FILE* mainStorage = NULL;
FILE* metadataFile = NULL;
AllocationTable* currentTable = NULL;
gboolean disk_initialized = FALSE;

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
void show_error_dialog(GtkWidget *parent, const char *message);
void show_success_dialog(GtkWidget *parent, const char *message);
void refresh_disk_view();
void on_draw_disk_clicked(GtkToolButton *button, gpointer user_data); // Add this line
void update_metadata_view(const char *metadata_text);
void update_file_tree_view();
// Create the file tree view

void load_existing_data() {
    mainStorage = fopen("storage.bin", "rb+");
    metadataFile = fopen("metadata.bin", "rb+");

    if (mainStorage && metadataFile) {
        // Try to read the allocation table
        currentTable = initAllocationTable();
        ReadAllocationTable(currentTable, mainStorage);
        if (currentTable  == 0) {
            disk_initialized = TRUE;
            refresh_disk_view();
            update_file_tree_view();
            return;
        }
    }

    // If we get here, either the files don't exist or are corrupt
    if (mainStorage) fclose(mainStorage);
    if (metadataFile) fclose(metadataFile);
    if (currentTable) free(currentTable);
    mainStorage = NULL;
    metadataFile = NULL;
    currentTable = NULL;
    disk_initialized = FALSE;
}

void on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data) {
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *filename;
        gtk_tree_model_get(model, &iter, 0, &filename, -1);  // Get filename from first column

        // Search for the metadata position
        int pos = search_metadata(filename, metadataFile);
        if (pos >= 0) {
            // Read the metadata
            Metadata meta;
            Readmeta_FULL(metadataFile, &meta, pos);

            // Create formatted metadata text
            char metadata_text[512];
            snprintf(metadata_text, sizeof(metadata_text),
                     "File: %s\n"
                     "First Block: %d\n"
                     "Number of Blocks: %d\n"
                     "Global Organization: %s\n"
                     "Internal Organization: %s\n"
                     "Number of Records: %d",
                     meta.filename,
                     meta.Firstblock,
                     meta.nBlocks,
                     meta.global_organs ? "LIST" : "SEQUENTIAL",
                     meta.inter_organs ? "Ordered" : "Unordered",
                     meta.nRecords);

            // Update the metadata view
            update_metadata_view(metadata_text);
        } else {
            update_metadata_view("No metadata found for selected file");
        }

        g_free(filename);
    }
}
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

    // Add selection handling
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(on_tree_selection_changed), NULL);

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

    gtk_container_add(GTK_CONTAINER(scroll), tree);

    return scroll;
}

// Update file tree view with actual file system data
void update_file_tree_view() {
    if (!metadataFile) return;

    GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(
            GTK_TREE_VIEW(gtk_bin_get_child(GTK_BIN(file_tree_view)))));

    gtk_tree_store_clear(store);

    // Read metadata file to list actual files
    fseek(metadataFile, 0, SEEK_END);
    long fileSize = ftell(metadataFile);
    rewind(metadataFile);

    while (ftell(metadataFile) < fileSize) {
        Metadata meta;
        if (fread(&meta, sizeof(Metadata), 1, metadataFile) == 1) {
            GtkTreeIter iter;
            gtk_tree_store_append(store, &iter, NULL);

            // Determine organization type
            // Placeholder for other modes
            const char* orgType;
            if (meta.global_organs == 1 && meta.inter_organs == 1) {
                orgType = "LOF";
            } else if (meta.global_organs == 1 && meta.inter_organs == 0) {
                orgType = "LNOF";
            } else if (meta.global_organs == 0 && meta.inter_organs == 1) {
                orgType = "TOF";
            } else{
                orgType = "TNOF";

            }


            gtk_tree_store_set(store, &iter,
                               0, meta.filename,
                               1, orgType,
                               -1);
        }
    }
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
    refresh_disk_view();
    update_file_tree_view();
}

void on_compact_disk_clicked(GtkToolButton *button, gpointer user_data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(
            GTK_TREE_VIEW(gtk_bin_get_child(GTK_BIN(file_tree_view))));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *filename;
        gtk_tree_model_get(model, &iter, 0, &filename, -1);

        TNOF_Reorganize(filename, mainStorage, metadataFile);

        refresh_disk_view();
        update_file_tree_view();

        g_free(filename);

        show_success_dialog(GTK_WIDGET(window), "File compacted successfully");
    } else {
        show_error_dialog(GTK_WIDGET(window), "Please select a file to compact");
    }
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
    if (!disk_initialized) {
        show_error_dialog(parent, "Please initialize disk first");
        return;
    }

    SearchRecordDialog dialog;
    dialog.dialog = gtk_dialog_new_with_buttons("Search Record",
                                                GTK_WINDOW(parent),
                                                GTK_DIALOG_MODAL,
                                                "Cancel", GTK_RESPONSE_CANCEL,
                                                "Search", GTK_RESPONSE_ACCEPT,
                                                NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog.dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_entry_set_text(GTK_ENTRY(dialog.id_entry), ""); // Clear text field
    // File selection combo
    GtkWidget *file_label = gtk_label_new("Select file:");
    dialog.file_combo = gtk_combo_box_text_new();

    // Populate combo box with existing files
    rewind(metadataFile);
    Metadata meta;
    while (fread(&meta, sizeof(Metadata), 1, metadataFile) == 1) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog.file_combo), meta.filename);
    }

    // Get selected file from tree view if any
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gtk_bin_get_child(GTK_BIN(file_tree_view))));
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *filename;
        gtk_tree_model_get(model, &iter, 0, &filename, -1);
        // Find and select this file in the combo box
        GtkTreeModel *combo_model = gtk_combo_box_get_model(GTK_COMBO_BOX(dialog.file_combo));
        GtkTreeIter combo_iter;
        gboolean valid = gtk_tree_model_get_iter_first(combo_model, &combo_iter);
        int index = 0;
        while (valid) {
            gchar *current;
            gtk_tree_model_get(combo_model, &combo_iter, 0, &current, -1);
            if (g_strcmp0(current, filename) == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(dialog.file_combo), index);
                g_free(current);
                break;
            }
            g_free(current);
            valid = gtk_tree_model_iter_next(combo_model, &combo_iter);
            index++;
        }
        g_free(filename);
    }

    // Record ID entry
    GtkWidget *id_label = gtk_label_new("Record ID:");
    dialog.id_entry = gtk_entry_new();

    // Add widgets to grid
    gtk_grid_attach(GTK_GRID(grid), file_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.file_combo, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.id_entry, 1, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog.dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog.dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        const char* filename = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(dialog.file_combo));
        const char* id_text = gtk_entry_get_text(GTK_ENTRY(dialog.id_entry));
        printf("ID Text from Entry: '%s'\n", id_text); // Debugging

        int record_id = atoi(id_text);
        if (filename && id_text) {
            printf("Converted Record ID: %d\n", record_id); // Debugging



            int meta_pos = search_metadata(filename, metadataFile);
            printf("Converted Record ID: %d\n", record_id); // Debugging

            if (meta_pos >= 0) {
                Metadata meta;
                Readmeta_FULL(metadataFile, &meta, meta_pos);

                // Search based on organization type
                coords result;
                if (meta.global_organs == 0 && meta.inter_organs == 0) {
                    printf("Converted STARTTT\n"); // Debugging

                    printf("%s", filename);
                    result = TNOF_SearchRecord(mainStorage, metadataFile, filename,record_id);
                    printf("%d %d", result.y_record, result.x_block);
                    if (result.found) {
                        char msg[256];
                        snprintf(msg, sizeof(msg), "Record found at block %d and record num %d", result.x_block, result.y_record);
                        show_success_dialog(parent, msg);
                    } else {
                        show_error_dialog(parent, "Record not found bruh");
                    }
                }
                // Add other organization types here

                refresh_disk_view();
                update_file_tree_view();
            }
        }
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
    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Rename File",
            GTK_WINDOW(parent),
            GTK_DIALOG_MODAL,
            "Cancel", GTK_RESPONSE_CANCEL,
            "Rename", GTK_RESPONSE_ACCEPT,
            NULL
    );

    // Create content area and grid
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    // File selection combo box
    GtkWidget *file_label = gtk_label_new("Select file:");
    GtkWidget *file_combo = gtk_combo_box_text_new();

    // Populate combo box with existing files
    rewind(metadataFile);
    Metadata meta;
    while (fread(&meta, sizeof(Metadata), 1, metadataFile) == 1) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(file_combo), meta.filename);
    }

    // New name entry
    GtkWidget *name_label = gtk_label_new("New name:");
    GtkWidget *name_entry = gtk_entry_new();

    // Add widgets to the grid
    gtk_grid_attach(GTK_GRID(grid), file_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), file_combo, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog);

    // Run dialog and handle response
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        const char *selected_file = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(file_combo));
        const char *new_name = gtk_entry_get_text(GTK_ENTRY(name_entry));

        // Validate input
        if (!selected_file || strlen(selected_file) == 0) {
            show_error_dialog(parent, "Please select a file.");
        } else if (!new_name || strlen(new_name) == 0) {
            show_error_dialog(parent, "Please enter a new name.");
        } else {
            // Perform rename operation
            if (RenameFile(selected_file, new_name, metadataFile)) {
                gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "File renamed successfully.");
            } else {
                show_error_dialog(parent, "Failed to rename the file.");
            }
        }
    }

    // Clean up
    gtk_widget_destroy(dialog);
}
//void show_all_records_dialog(GtkWidget *parent) {
//    if (!disk_initialized) {
//        show_error_dialog(parent, "Please initialize disk first");
//        return;
//    }
//
//    GtkWidget *dialog = gtk_dialog_new_with_buttons("All Records",
//                                                    GTK_WINDOW(parent),
//                                                    GTK_DIALOG_MODAL,
//                                                    "Close",
//                                                    GTK_RESPONSE_CLOSE,
//                                                    NULL);
//
//    // Create scrolled window
//    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
//    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
//    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
//                                   GTK_POLICY_AUTOMATIC,
//                                   GTK_POLICY_AUTOMATIC);
//
//    // Create tree view
//    GtkListStore *store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_BOOLEAN);
//    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
//
//    // Add columns
//    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
//    gtk_tree_view_append_column(GTK_TREE_VIEW(tree),
//                                gtk_tree_view_column_new_with_attributes("ID",
//                                                                         renderer,
//                                                                         "text", 0,
//                                                                         NULL));
//
//    renderer = gtk_cell_renderer_text_new();
//    gtk_tree_view_append_column(GTK_TREE_VIEW(tree),
//                                gtk_tree_view_column_new_with_attributes("Content",
//                                                                         renderer,
//                                                                         "text", 1,
//                                                                         NULL));
//
//    renderer = gtk_cell_renderer_toggle_new();
//    gtk_tree_view_append_column(GTK_TREE_VIEW(tree),
//                                gtk_tree_view_column_new_with_attributes("Deleted",
//                                                                         renderer,
//                                                                         "active", 2,
//                                                                         NULL));
//
//    // Get selected file and show records
//    GtkTreeSelection *selection = gtk_tree_view_get_selection(
//            GTK_TREE_VIEW(gtk_bin_get_child(GTK_BIN(file_tree_view))));
//    GtkTreeModel *model;
//    GtkTreeIter iter;
//
//    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
//        gchar *filename;
//        gtk_tree_model_get(model, &iter, 0, &filename, -1);
//
//        // Read records and populate tree view
//        int meta_pos = search_metadata(filename, metadataFile);
//        if (meta_pos >= 0) {
//            Metadata meta;
//            Readmeta_FULL(metadataFile, &meta, meta_pos);
//            Block current_block;
//            current_block.tab =  (Record*)malloc(blocking_fact * sizeof(Record)); // Allocate memory for records
//
//            fseek(mainStorage, sizeof(current_block) * (meta.Firstblock + meta.nBlocks), SEEK_SET);
//            for (int block = 0; block < meta.nBlocks; block++) {
//                Block current_block;
//                ReadBlock(mainStorage, *t_block, block);
//
//                for (int i = 0; i < blocking_fact; i++) {
//                    Record *record = ¤t_block.records[i];
//                    GtkTreeIter list_iter;
//                    gtk_list_store_append(store, &list_iter);
//                    gtk_list_store_set(store, &list_iter,
//                                       0, record->id,
//                                       1, record->content,
//                                       2, record->is_deleted,
//                                       -1);
//                }
//            }
//        }
//        g_free(filename);
//    }
//
//    // Show dialog
//    gtk_container_add(GTK_CONTAINER(scroll), tree);
//    gtk_widget_set_size_request(scroll, 400, 300);
//    gtk_container_add(GTK_CONTAINER(content_area), scroll);
//    gtk_widget_show_all(dialog);
//
//    gtk_dialog_run(GTK_DIALOG(dialog));
//    gtk_widget_destroy(dialog);
//}
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
    GtkToolItem *compact_button = gtk_tool_button_new(NULL, "Compact File");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), compact_button, -1);
    g_signal_connect(compact_button, "clicked", G_CALLBACK(on_compact_disk_clicked), NULL);

// Add a toolbar button for show all records functionality
    GtkToolItem *records_button = gtk_tool_button_new(NULL, "Show Records");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), records_button, -1);
//    g_signal_connect(records_button, "clicked", G_CALLBACK(show_all_records_dialog), NULL);
    // Clear Disk
    GtkToolItem *clear_button = gtk_tool_button_new(NULL, "Clear");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), clear_button, -1);
    g_signal_connect(clear_button, "clicked", G_CALLBACK(on_clear_disk_clicked), NULL);

    // Draw Disk
    GtkToolItem *draw_button = gtk_tool_button_new(NULL, "Draw Disk");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), draw_button, -1);
    g_signal_connect(draw_button, "clicked", G_CALLBACK(on_draw_disk_clicked), NULL);

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
    if (!mainStorage || !currentTable) return FALSE;

    int current_row = 0;
    int current_col = 0;

    for (int i = 0; i < num_of_blocks; i++) {
        int x = BLOCK_SPACING + current_col * (BLOCK_SIZE + BLOCK_SPACING);
        int y = BLOCK_SPACING + current_row * (BLOCK_SIZE + BLOCK_SPACING);

        // Color based on actual allocation status
        if (currentTable->arrays[i] == 0) {
            cairo_set_source_rgb(cr, 0.2, 0.8, 0.2); // Free block (green)
        } else {
            cairo_set_source_rgb(cr, 0.8, 0.2, 0.2); // Used block (red)
        }

        // Draw block
        cairo_rectangle(cr, x, y, BLOCK_SIZE, BLOCK_SIZE);
        cairo_fill(cr);

        // Add block number
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 10);
        char block_text[32];
        snprintf(block_text, sizeof(block_text), "Block %d", i);
        cairo_move_to(cr, x + 5, y + 20);
        cairo_show_text(cr, block_text);

        // Update position
        current_col++;
        if (current_col >= BLOCKS_PER_ROW) {
            current_col = 0;
            current_row++;
        }
    }

    return FALSE;
}
void cleanup() {
    if (mainStorage) {
        fflush(mainStorage);
        fclose(mainStorage);
        mainStorage = NULL;
    }
    if (metadataFile) {
        fflush(metadataFile);
        fclose(metadataFile);
        metadataFile = NULL;
    }
    if (currentTable) {
        free(currentTable);
        currentTable = NULL;
    }
    gtk_main_quit();
}

// Function to create and show the initialize disk dialog
void show_init_disk_dialog(GtkWidget *parent) {
    InitDiskDialog dialog;
    dialog.dialog = gtk_dialog_new_with_buttons("Initialize Disk",
                                                GTK_WINDOW(parent),
                                                GTK_DIALOG_MODAL,
                                                "Cancel", GTK_RESPONSE_CANCEL,
                                                "Initialize", GTK_RESPONSE_ACCEPT,
                                                NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog.dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    // Number of blocks
    GtkWidget *blocks_label = gtk_label_new("Number of blocks:");
    dialog.blocks_spin = gtk_spin_button_new_with_range(1, 1000, 1); // Min: 1, Max: 1000, Step: 1
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.blocks_spin), 100); // Default value

    // Blocking factor (records per block)
    GtkWidget *blocking_factor_label = gtk_label_new("Blocking factor (records per block):");
    dialog.block_size_spin = gtk_spin_button_new_with_range(1, 100, 1); // Min: 1, Max: 100, Step: 1
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog.block_size_spin), 10); // Default value

    // Add widgets to the grid
    gtk_grid_attach(GTK_GRID(grid), blocks_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.blocks_spin, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), blocking_factor_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dialog.block_size_spin, 1, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog.dialog);

    // Run the dialog and handle the response
    gint result = gtk_dialog_run(GTK_DIALOG(dialog.dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        // Get the values from the dialog
        num_of_blocks = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dialog.blocks_spin));
        blocking_fact = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dialog.block_size_spin));

        // Close existing files if they are open
        if (mainStorage) {
            fclose(mainStorage);
            mainStorage = NULL;
        }
        if (metadataFile) {
            fclose(metadataFile);
            metadataFile = NULL;
        }

        // Create new files with proper permissions
        mainStorage = fopen("storage.bin", "wb+");
        metadataFile = fopen("metadata.bin", "wb+");

        if (!mainStorage || !metadataFile) {
            show_error_dialog(parent, "Failed to create storage files. Check permissions.");
            if (mainStorage) fclose(mainStorage);
            if (metadataFile) fclose(metadataFile);
            mainStorage = metadataFile = NULL;
            gtk_widget_destroy(dialog.dialog);
            return;
        }

        // Initialize files with zero bytes
        char zero = 0;
        for (int i = 0; i < num_of_blocks * blocking_fact * sizeof(Record); i++) {
            fwrite(&zero, 1, 1, mainStorage);
        }
        fflush(mainStorage);

        // Initialize allocation table
        currentTable = initAllocationTable();
        WriteAllocationTable(currentTable, mainStorage);

        disk_initialized = TRUE;
        refresh_disk_view();
        update_file_tree_view();

        show_success_dialog(parent, "Disk initialized successfully");
    }
    gtk_widget_destroy(dialog.dialog);
}
// Function to create and show the create file dialog
void show_create_file_dialog(GtkWidget *parent) {
    CreateFileDialog dialog;

    if (!mainStorage || !metadataFile || !currentTable) {
        show_error_dialog(parent, "Please initialize disk first");
        return;
    }
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
        const char* filename = gtk_entry_get_text(GTK_ENTRY(dialog.name_entry));
        int nrecords = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dialog.records_spin));

        // Only handle TNOF mode for now

        TNOF_InitliazeFile(mainStorage, metadataFile, filename, nrecords, currentTable);

        gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "File created");
        refresh_disk_view();
        update_file_tree_view();
    }

    gtk_widget_destroy(dialog.dialog);
}
gboolean auto_save(gpointer data) {
    if (mainStorage && metadataFile) {
        fflush(mainStorage);
        fflush(metadataFile);
    }
    return TRUE;
}

// In main():
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
    g_timeout_add(5000, auto_save, NULL); // Save every 5 seconds

    // Start the GTK main loop
    load_existing_data();
    g_signal_connect(window, "destroy", G_CALLBACK(cleanup), NULL);

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
void on_draw_disk_clicked(GtkToolButton *button, gpointer user_data) {
    // Toggle the visibility of the disk view
    gboolean is_visible = gtk_widget_get_visible(disk_view_drawing_area);
    show_disk_view(!is_visible);

    // Refresh the disk view if it's being shown
    if (!is_visible) {
        refresh_disk_view();
    }

    // Update the status bar
    if (!is_visible) {
        gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "Disk view shown");
    } else {
        gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "Disk view hidden");
    }
}



// Create the file tree view
