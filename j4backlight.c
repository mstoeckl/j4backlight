#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

const char* max_path = "/sys/class/backlight/intel_backlight/max_brightness";
const char* target_path = "/sys/class/backlight/intel_backlight/brightness";
const char* usage = "Usage: j4backlight + | j4backlight - | j4backlight [1..%d]\n";

const int steps = 30;

static int read_from_file_or_die(const char* file) {
    FILE* f = fopen(file, "r");
    if (!f) {
        printf("Could not open file to read %s\n", file);
        exit(1);
    }
    char buf[256];
    fread(buf, 1, 256, f);
    fclose(f);
    char* end = 0;
    int val = strtol(buf, &end, 10);
    if (end == buf) {
        printf("Could not interpret value from %s\n", file);
        exit(1);
    }
    return val;
}

static int get_max_brightness_or_die() {
    return read_from_file_or_die(max_path);
}

static int get_current_brightness_or_die() {
    return read_from_file_or_die(target_path);
}

static void set_current_brightness(int value, int previous) {

    FILE* f = fopen(target_path, "w");
    if (!f) {
        printf("Could not open file to write %d to %s\n", value, target_path);
        exit(1);
    }
    if (fprintf(f, "%d", value) < 0) {
        printf("Could not write %d to file %s\n", value, target_path);
        exit(1);
    }
    fclose(f);
    printf("Setting brightness to %d (was %d)\n", value, previous);
}

static float get_base(int steps, int max) {
    return pow(max, 1.0/(steps - 1));
}

int main(int argc, const char** argv) {
    if (argc == 1) {
        int current = get_current_brightness_or_die();
        printf("%d\n", current);
        return 0;
    }
    const int max_brightness = get_max_brightness_or_die();
    if (argc != 2) {
        printf(usage, max_brightness);
        return 1;
    }
    
    const char* arg = argv[1];
    if (strcmp(arg, "-") == 0) {
        // Decrease brightness to nearest level
        double base = get_base(steps, max_brightness);
        const int current = get_current_brightness_or_die();
        int last = 0;
        // yes I know, bad algo
        for (int k=0;k<steps;k++) {
            int next = lround(pow(base, (double)k));
            if (next >= current) {
                break;
            }
            last = next;
            if (k == steps - 1) {
                last = max_brightness;
            }
        }
        set_current_brightness(last, current);
        return 0;
    } else if (strcmp(arg, "+") == 0) {
        // Increase brightness to nearest level
        float base = get_base(steps, max_brightness);
        const int current = get_current_brightness_or_die();
        int last = max_brightness;
        for (int k=steps - 1;k>0;k--) {
            int next = lround(pow(base, (double)k));
            if (next <= current) {
                break;
            }
            last = next;
            if (k == 1) {
                last = 1;
            }
        }
        set_current_brightness(last, current);
        return 0;
    } else {
        char* end = 0;
        int val = strtol(arg, &end, 10);
        if (end == arg) {
            printf(usage, max_brightness);
            return 1;
        }
        if (val < 1 || val > max_brightness) {
            printf("Value %d out of range [1..%d]. No action taken.\n", val, max_brightness);
            return 1;
        }
        set_current_brightness(val, get_current_brightness_or_die());
        return 0;
    }
}
    
