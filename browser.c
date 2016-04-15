#include <stdio.h>
#include <signal.h>

#include <gtk/gtk.h>
#include <webkit/webkitdom.h>
#include <gdk/gdkkeysyms.h>
#include <webkit/webkit.h>

gboolean on_key_press(GtkWidget*, GdkEventKey*, gpointer);

void reload_browser(int);
void toggle_fullscreen(int);
void maximize();
void unmaximize();

static WebKitWebView* web_view;
static GtkWidget *window;

int main(int argc, char** argv) {
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());

  signal(SIGHUP, reload_browser);
  signal(SIGUSR1, toggle_fullscreen);

  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(web_view));

  if(argc > 1) {
    webkit_web_view_load_uri(web_view, argv[1]);
  }
  else {
    webkit_web_view_load_uri(web_view, default_url);
  }

  gtk_window_set_default_size (window, 1920, 1080);
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtk_widget_show_all(window);
  gtk_main();

  web_view_get_link_url(web_view, "login");   

  return 0;
}

static voidweb_view_javascript_finished (GObject      *object, GAsyncResult *result, gpointer      user_data)
{
    WebKitJavascriptResult *js_result;
    JSValueRef              value;
    JSGlobalContextRef      context;
    GError                 *error = NULL;

    js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
    if (!js_result) {
        g_warning ("Error running javascript: %s", error->message);
        g_error_free (error);
        return;
    }

    context = webkit_javascript_result_get_global_context (js_result);
    value = webkit_javascript_result_get_value (js_result);
    if (JSValueIsString (context, value)) {
        JSStringRef js_str_value;
        gchar      *str_value;
        gsize       str_length;

        js_str_value = JSValueToStringCopy (context, value, NULL);
        str_length = JSStringGetMaximumUTF8CStringSize (js_str_value);
        str_value = (gchar *)g_malloc (str_length);
        JSStringGetUTF8CString (js_str_value, str_value, str_length);
        JSStringRelease (js_str_value);
        g_print ("Script result: %s\n", str_value);
        g_free (str_value);
    } else {
        g_warning ("Error running javascript: unexpected return value");
    }
    webkit_javascript_result_unref (js_result);
}

static void web_view_get_link_url (WebKitWebView *web_view, const gchar   *link_id)
{
    gchar *script;

    script = g_strdup_printf ("window.document.getElementById('%s').action;", link_id);
    webkit_web_view_run_javascript (web_view, script, NULL, web_view_javascript_finished, NULL);
    g_free (script);
}

gboolean on_key_press(GtkWidget* window, GdkEventKey* key, gpointer userdata) {
  if(key->type == GDK_KEY_PRESS && key->keyval == GDK_F5) {
    reload_browser(0);
  }
  else if(key->type == GDK_KEY_PRESS && key->keyval == GDK_F11) {
    toggle_fullscreen(0);
  }

  return FALSE;
}

void reload_browser(int signum) {
  webkit_web_view_reload_bypass_cache(web_view);
}

void toggle_fullscreen(int signum) {
  if(gtk_window_get_decorated(GTK_WINDOW(window))) {
    maximize();
  }
  else {
    unmaximize();
  }
}

void maximize() {
  gtk_window_maximize(GTK_WINDOW(window));
  gtk_window_fullscreen(GTK_WINDOW(window));
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
}

void unmaximize() {
  gtk_window_unmaximize(GTK_WINDOW(window));
  gtk_window_unfullscreen(GTK_WINDOW(window));
  gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
  gtk_window_resize(GTK_WINDOW(window), 1280, 768);
}
