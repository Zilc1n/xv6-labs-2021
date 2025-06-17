#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// xv6 默认每秒有 100 个时钟滴答 (ticks)
#define HZ 100
// 定义要执行的交换次数
#define ROUNDS 10000

int
main(int argc, char *argv[])
{
    // p_to_c: 父进程 -> 子进程 ("ping")
    // c_to_p: 子进程 -> 父进程 ("pong")
    int p_to_c[2], c_to_p[2];
    char buf[1] = {'a'}; // 用来传递的字节
    int pid;
    int i;
    int start_time, end_time;

    // 1. 创建两个管道
    if (pipe(p_to_c) < 0 || pipe(c_to_p) < 0) {
        fprintf(2, "pingpong: pipe failed\n");
        exit(1);
    }

    // 2. 创建子进程
    pid = fork();

    if (pid < 0) {
        fprintf(2, "pingpong: fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // --- 子进程代码 ---

        // 3a. 关闭不需要的管道端口
        // 子进程从 p_to_c 读取, 向 c_to_p 写入
        close(p_to_c[1]); // 关闭 p_to_c 的写入端
        close(c_to_p[0]); // 关闭 c_to_p 的读取端

        for (;;) {
            // 等待来自父进程的 "ping"
            // 当父进程关闭管道后, read会返回0, 循环终止
            if (read(p_to_c[0], buf, 1) != 1) {
                break; // 父进程已退出
            }

            // 发送 "pong" 回给父进程
            if (write(c_to_p[1], buf, 1) != 1) {
                break; // 写入失败
            }
        }

        // 清理并退出
        close(p_to_c[0]);
        close(c_to_p[1]);
        exit(0);

    } else {
        // --- 父进程代码 ---

        // 3b. 关闭不需要的管道端口
        // 父进程向 p_to_c 写入, 从 c_to_p 读取
        close(p_to_c[0]); // 关闭 p_to_c 的读取端
        close(c_to_p[1]); // 关闭 c_to_p 的写入端

        // 4. 测量性能
        start_time = uptime();

        for (i = 0; i < ROUNDS; i++) {
            // 发送 "ping" 到子进程
            if (write(p_to_c[1], buf, 1) != 1) {
                fprintf(2, "pingpong: parent write failed\n");
                exit(1);
            }

            // 等待来自子进程的 "pong"
            if (read(c_to_p[0], buf, 1) != 1) {
                fprintf(2, "pingpong: parent read failed\n");
                exit(1);
            }
        }

        end_time = uptime();

        // 5. 打印结果
        int duration = end_time - start_time;
        printf("Ping-pong test completed.\n");
        printf("Total round trips: %d\n", ROUNDS);
        printf("Total time: %d ticks\n", duration);
        
        // 避免除以零
        if (duration > 0) {
            // 性能 = (总次数 * 每秒ticks) / 总ticks
            int performance = (ROUNDS * HZ) / duration;
            printf("Performance: %d exchanges/sec\n", performance);
        } else {
            printf("Performance: Measurement too fast!\n");
        }


        // 6. 清理并退出
        // 关闭管道会向子进程的read发送EOF信号，使其退出
        close(p_to_c[1]);
        close(c_to_p[0]);

        // 等待子进程完全退出
        wait(0);
        exit(0);
    }
}