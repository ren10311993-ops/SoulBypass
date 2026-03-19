#!/sbin/sh

# Custom installation script

ui_print "========================================"
ui_print "  SoulBypass v1.0.0"
ui_print "  Zygisk Module for Soul App"
ui_print "========================================"
ui_print ""
ui_print "Target: Soul App v6.10.0"
ui_print "Architecture: arm64-v8a only"
ui_print ""
ui_print "Features:"
ui_print "  - Seccomp-BPF syscall interception"
ui_print "  - PC tracing for libIncite.so"
ui_print "  - 19 sensitive path filters"
ui_print "  - memfd-based file spoofing"
ui_print ""
ui_print "Installing..."

# Check architecture
if [ "$ARCH" != "arm64" ]; then
    ui_print "[!] Error: This module only supports arm64-v8a"
    ui_print "[!] Your architecture: $ARCH"
    abort "Installation aborted"
fi

# Check Android version
if [ "$API" -lt 29 ]; then
    ui_print "[!] Warning: Android 10+ (API 29+) required"
    ui_print "[!] Your API level: $API"
fi

ui_print "[+] Architecture check passed: $ARCH"
ui_print "[+] API level: $API"

# Extract module
ui_print "[+] Extracting module files..."
unzip -o "$ZIPFILE" -d "$MODPATH" >&2

# Set permissions
ui_print "[+] Setting permissions..."
set_perm_recursive "$MODPATH" 0 0 0755 0644
set_perm_recursive "$MODPATH/zygisk" 0 0 0755 0755

ui_print ""
ui_print "[+] Installation complete!"
ui_print ""
ui_print "Please reboot to activate"
ui_print "========================================"
