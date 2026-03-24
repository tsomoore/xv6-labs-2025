#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h" // 包含 MAXARG 的定义

// 存储全局的 exec 状态，方便递归函数访问
char *exec_argv[MAXARG];
int exec_argc = 0;
int has_exec = 0;

// 辅助函数：提取路径最后的文件名
char* get_fname(char *path) {
  char *p;
  for (p = path + strlen(path); p >= path && *p != '/'; p--);
  return p + 1;
}

// 执行联动的命令
void run_exec(char *path) {
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "find: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    // 子进程：组装最终的参数列表
    // 假设命令是: echo hi -> argv 变成: ["echo", "hi", "path", 0]
    char *cmd[MAXARG];
    for (int i = 0; i < exec_argc; i++) {
      cmd[i] = exec_argv[i];
    }
    cmd[exec_argc] = path; // 将找出的路径作为最后一个参数
    cmd[exec_argc + 1] = 0; // NULL 结尾

    exec(cmd[0], cmd);
    fprintf(2, "find: exec %s failed\n", cmd[0]);
    exit(1);
  } else {
    // 父进程：必须等待子进程结束，否则输出会混乱，且会产生僵尸进程
    wait(0);
  }
}

void find(char *path, char *target) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, 0)) < 0) return;
  if (fstat(fd, &st) < 0) { close(fd); return; }

  // 1. 匹配检查
  if (strcmp(get_fname(path), target) == 0) {
    if (has_exec) {
      run_exec(path); // 执行 -exec 逻辑
    } else {
      printf("%s\n", path); // 默认打印
    }
  }

  // 2. 递归遍历目录
  if (st.type == T_DIR) {
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      close(fd);
      return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) continue;

      // 安全拷贝文件名并封口
      char name[DIRSIZ + 1];
      memmove(name, de.name, DIRSIZ);
      name[DIRSIZ] = 0;

      // 绝对不能递归进入 . 和 ..
      if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

      memmove(p, name, strlen(name));
      p[strlen(name)] = 0;
      find(buf, target);
    }
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(2, "usage: find <path> <target> [-exec <cmd> <args...>]\n");
    exit(1);
  }

  char *path = argv[1];
  char *target = argv[2];

  // 解析 -exec 参数
  for (int i = 3; i < argc; i++) {
    if (strcmp(argv[i], "-exec") == 0) {
      has_exec = 1;
      // 收集 -exec 之后的所有参数作为命令模板
      for (int j = i + 1; j < argc; j++) {
        if (exec_argc < MAXARG - 2) {
          exec_argv[exec_argc++] = argv[j];
        }
      }
      break;
    }
  }

  find(path, target);
  exit(0);
}