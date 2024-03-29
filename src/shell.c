#include "shell.h"
#include "types.h"
#include "math.h"
#include "font.h"
#include "gfx.h"
#include "string.h"
#include "fb.h"
#include "printf.h"
#include "mini_uart.h"
#include "schedule.h"
#include "utils.h"

#define print_console printf
#define print_console_c uart_putc


#define CMD_BUFFER_SIZE 128
#define MAX_SHELL_ARGS 16

extern u64 scheduler_ticks_per_second;

#define NUM_CMDS 3
struct command shell_cmds[NUM_CMDS] = {
    { "ptable", NULL, print_ptable },
    { "kill", "<pid>", kill },
    { "help", NULL, help }
};

#define NUM_PROGS 5
struct command shell_progs[NUM_PROGS] = {
    { "pi_logo", "<x> <y>", draw_pi_logo },
    { "test_loop", "<ms delay>", test_loop },
    { "colors", "<x> <y>", rainbow_square },
    { "uptime", NULL, uptime },
    { "clear", NULL, clear_framebuffer }
};

void clear_framebuffer(int argc, char **argv) {
    fbclear(0);
}

void test_loop(int argc, char **argv) {
    int delay = 2000;
    if(argc > 1)
        delay = strtol(argv[1]);
    int i=0;
    u64 pid = get_pid();
    while(1) {
        u8 core = get_core();
        print_console("\ncore %d pid %d iter %d\n> ", core, pid, i++);
        sys_timer_sleep_ms(delay);
    }
}

void uptime(int argc, char **argv) {
    u64 uptime_ms = sys_timer_get_ms();
    u64 uptime_s = uptime_ms / 1000;
    u64 uptime_m = uptime_s / 60;
    u64 uptime_h = uptime_m / 60;
    u64 uptime_d = uptime_h / 24;
    print_console("\nuptime: %dd %dh %dm %ds %dms\n> ", uptime_d, uptime_h % 24, uptime_m % 60, uptime_s % 60, uptime_ms % 1000);
}

void help(int argc, char **argv) {
    print_console("Here are the available commands:\n");
    for(int i=0;i<NUM_CMDS;++i) {
        print_console("  %s %s\n", shell_cmds[i].name, shell_cmds[i].arghint == 0 ? "" : shell_cmds[i].arghint);
    }
    print_console("\nHere are the available programs:\n");
    for(int i=0;i<NUM_PROGS;++i) {
        print_console("  %s %s\n", shell_progs[i].name, shell_progs[i].arghint == 0 ? "" : shell_progs[i].arghint);
    }
    print_console("\n");
}

void parse_command(char * commandbuffer, char **args) {
    int argc=0;
    char *argstart = commandbuffer;

    char * cur = commandbuffer;
    while(*cur)
    {
        if(argc > MAX_SHELL_ARGS) return;
        if(*cur == ' ')
        {
            args[argc++] = argstart;
            *cur = 0;
            argstart = ++cur;
        } else ++cur;
    }
    args[argc++] = argstart;
    args[argc] = 0;
    
    for(int n=0;n<NUM_CMDS;++n) {
        if(strcmp(args[0], shell_cmds[n].name) == 0)
        {
            void (*fun_ptr)(int, char**) =  shell_cmds[n].entry;
            fun_ptr(argc, args);
        }
    }

    for(int n=0;n<NUM_PROGS;++n) {
        if(strcmp(args[0], shell_progs[n].name) == 0)
        {
            new_process((u64) shell_progs[n].entry, shell_progs[n].name, argc, args);
        }
    }
}

static char commandbuffer[CMD_BUFFER_SIZE];
static char args[MAX_SHELL_ARGS][CMD_BUFFER_SIZE];

void shell() {
    for(int i=0;i<CMD_BUFFER_SIZE;++i) {
        commandbuffer[i] = 0;
    }

    for(int i=0;i<MAX_SHELL_ARGS;++i) {
        for(int j=0;j<CMD_BUFFER_SIZE;++j) {
            args[i][j] = 0;
        }
    }
    u8 core = get_core();

    u32 ci = 0;
    printf("\nshell\n> ");
    while(1) {
        char c = uart_getc();
        switch(c) {
            case 0x7F:
                print_console("Got 0x7F (backspace)\n");
                /*fb.cursor_x -= char_width;
                *(--cur) = 0;
                --cur;
                print_console_c(' ');
                fb.cursor_x -= char_width;
                /break;*/
                break;
            case '\r':
                commandbuffer[ci] = 0;
                ci = 0;
                uart_putc('\n');
                parse_command(commandbuffer, args);
                uart_puts("> ");
                break;
            default:
                print_console_c(c);
                commandbuffer[ci++] = c;
                break;
        }
    }
}