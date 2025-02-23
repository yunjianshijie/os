#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int judge(struct stat *file_stat) {

  return (file_stat->st_mode & S_IXUSR) || (file_stat->st_mode & S_IXGRP) ||
         (file_stat->st_mode & S_IXOTH);
}

void printf_color(const char *filename, struct stat *file_stat) {
  if (S_ISDIR(file_stat->st_mode)) {
    // 蓝色目录
    printf("%s%s%s  ", "\033[34m", filename, "\033[0m");
  } else if (judge(file_stat)) {
    // 红色可执行文件
    printf("%s%s%s  ", "\033[31m", filename, "\033[0m");
  } else if (S_ISLNK(file_stat->st_mode)) {
    // 绿色链接
    printf("%s%s%s  ", "\033[32m", filename, "\033[0m");
  } else {
    // 普通文件显示白色
    printf("%s%s%s  ", "\033[37m", filename, "\033[0m");
  }
}

// -l
void printf_long(const char *file_name, const struct stat *file_stat, int ls_i,
                 int ls_s) {
  if (ls_i) {
    printf("%lu ", file_stat->st_ino);
  }
  char arr[10];
  arr[0] = S_ISDIR(file_stat->st_mode) ? 'd' : '-';
  if (arr[0] == "-") {
    arr[0] = S_ISREG(file_stat->st_mode) ? '-' : 'l';
  }
  arr[1] = S_IRUSR & file_stat->st_mode ? 'r' : '-';
  arr[2] = S_IWUSR & file_stat->st_mode ? 'w' : '-';
  arr[3] = S_IXUSR & file_stat->st_mode ? 'x' : '-';
  arr[4] = S_IRGRP & file_stat->st_mode ? 'r' : '-';
  arr[5] = S_IWGRP & file_stat->st_mode ? 'w' : '-';
  arr[6] = S_IXGRP & file_stat->st_mode ? 'x' : '-';
  arr[7] = S_IROTH & file_stat->st_mode ? 'r' : '-';
  arr[8] = S_IWOTH & file_stat->st_mode ? 'w' : '-';
  arr[9] = S_IXOTH & file_stat->st_mode ? 'x' : '-';
  printf("%s", arr);
  printf(" %lu ", file_stat->st_nlink);

  // printf("%d ", file_stat->st_uid);
  // printf("%d ",file_stat->st_gid);
  uid_t uid = getuid();
  gid_t gid = getgid();
  // printfid(uid,gid);
  struct passwd *useid = getpwuid(uid);
  printf("%s ", useid->pw_name);
  struct group *groupid = getgrgid(gid);
  printf("%s ", groupid->gr_name);

  if (ls_s) {
    printf("%ld ", (long)file_stat->st_size);
  }

  char time_str[20];
  strftime(time_str, sizeof(time_str), "%m月 %d %H:%M",
           localtime(&(file_stat->st_atime)));
  printf("%s ", time_str);

  printf("%s\n", file_name);
}
//-l结束
//-r
void sort_r(struct dirent **file_list, int num_files) {
  for (int i = 0; i < num_files / 2; i++) {
    struct dirent *temp = file_list[i];
    file_list[i] = file_list[num_files - 1 - i];
    file_list[num_files - 1 - i] = temp;
  }
}

//-t
int sort_time(const void *a, const void *b) {
  struct dirent *entry_a = *((struct dirent **)a);
  struct dirent *entry_b = *((struct dirent **)b);

  char path_a[1024], path_b[1024];
  struct stat stat_a, stat_b;

  snprintf(path_a, 1024, "%s/%s", ".", entry_a->d_name);
  snprintf(path_b, 1024, "%s/%s", ".", entry_b->d_name);

  if (stat(path_a, &stat_a) == -1) {
    perror("stat");
    return 0;
  }
  if (stat(path_b, &stat_b) == -1) {
    perror("stat");
    return 0;
  }
  if (stat_a.st_mtime < stat_b.st_mtime) {
    return 1;
  } else if (stat_a.st_mtime > stat_b.st_mtime) {
    return -1;
  } else {
    return 0;
  }
}
//-t结束

void list(const char *path, int ls_all, int ls_long, int ls_R, int reverse,
          int time, int ls_i, int ls_s) {
  struct dirent *entry;
  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    return;
  }

  int sum_blocks = 0;
  char full_path[1024];
  struct stat file_stat;

  struct dirent **file_list = NULL; // 文件名数组
  int num_files = 0;

  while ((entry = readdir(dir)) != NULL) {
    if (!ls_all && entry->d_name[0] == '.') {
      continue;
    }

    snprintf(full_path, 1024, "%s/%s", path, entry->d_name);
    if (stat(full_path, &file_stat) == -1) {
      perror("stat");
      continue;
    }

    file_list = realloc(file_list, sizeof(struct dirent *) * (num_files + 1));
    file_list[num_files++] = entry;
  }

  if (time) {
    qsort(file_list, num_files, sizeof(struct dirent *), sort_time);
    // return;
  }

  if (reverse) {
    sort_r(file_list, num_files);
    // return;
  }

  for (int i = 0; i < num_files; i++) {
    snprintf(full_path, 1024, "%s/%s", path, file_list[i]->d_name);
    if (stat(full_path, &file_stat) == -1) {
      perror("stat");
      continue;
    }

    if (ls_long) {
      printf_long(file_list[i]->d_name, &file_stat, ls_i, ls_s);
    }
    if (ls_i) {
      //     printf("%s  ", file_list[i]->d_name);
      //  printf("%lu ", file_stat->st_ino);
      char file_path[1024];
      snprintf(file_path, sizeof(file_path), "%s/%s", path,
               file_list[i]->d_name);
      if (stat(file_path, &file_stat) == 0) {
        printf("%ld ", (long)file_stat.st_ino);
      }
      //   printf("%-15s   ",file_list[i]->d_name);
    }
    if (ls_s) {
      char file_path[1024];
      snprintf(file_path, sizeof(file_path), "%s/%s", path,
               file_list[i]->d_name);
      if (stat(file_path, &file_stat) == 0) {
        printf("%d ", file_stat.st_blocks / 2);
        sum_blocks += file_stat.st_blocks / 2;
      }
      //  printf("%-4s   ",file_list[i]->d_name);
    }

    if (!ls_long) {
      printf_color(file_list[i]->d_name, &file_stat);
    }

    //-R
    if (ls_R && S_ISDIR(file_stat.st_mode)) {
      printf("\n%s:\n", full_path);
      list(full_path, ls_all, ls_long, ls_R, reverse, time, ls_i, ls_s);
    }
  }
  if (ls_s) {
    printf("\n总计 %d\n", sum_blocks);
  }
  printf("\n");
  closedir(dir);
}

int main(int argc, char *argv[]) {
  int ls_all = 0;
  int ls_long = 0;
  int ls_R = 0;
  int reverse = 0;
  int time = 0;
  int ls_i = 0;
  int ls_s = 0;
  char *path = ".";

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (int j = 1; argv[i][j] != '\0'; j++) {
        switch (argv[i][j]) {
        case 'a':
          ls_all = 1;
          break;
        case 'l':
          ls_long = 1;
          ls_s = 1;
          break;
        case 'R':
          ls_R = 1;
          break;
        case 'r':
          reverse = 1;
          break;
        case 't':
          time = 1;
          break;
        case 'i':
          ls_i = 1;
          break;
        case 's':
          ls_s = 1;
          break;
        }
      }
    } else {
      path = argv[i];
    }
  }

  list(path, ls_all, ls_long, ls_R, reverse, time, ls_i, ls_s);

  return 0;
}