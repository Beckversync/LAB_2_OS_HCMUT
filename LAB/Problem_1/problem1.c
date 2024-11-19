#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>  // Thêm dòng này để khai báo waitpid
#include <unistd.h>

#define MAX_LINE_LENGTH 100

// Cấu trúc dữ liệu dùng trong bộ nhớ chia sẻ
struct shared_data {
    float total_rating_1;
    int count_1;
    float total_rating_2;
    int count_2;
};

// Hàm tính toán trung bình cho mỗi tệp
void compute_average(const char* filename, float* total_rating, int* count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Không thể mở tệp");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    int movieID, userID;
    float rating;
    *total_rating = 0.0;
    *count = 0;

    // Đọc tệp và tính tổng điểm và số lượng đánh giá
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d\t%d\t%f\t", &userID, &movieID, &rating) == 3) {
            *total_rating += rating;
            (*count)++;
        } else {
            fprintf(stderr, "Dòng không hợp lệ: %s\n", line);
        }
    }

    fclose(file);
}

int main() {
    // Tạo khóa và nhận ID bộ nhớ chia sẻ
    key_t key = ftok("movie-100k_1.txt", 65); // Tạo khóa duy nhất từ tệp
    int shm_id = shmget(key, sizeof(struct shared_data), 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("Không thể tạo bộ nhớ chia sẻ");
        exit(1);
    }

    // Gắn bộ nhớ chia sẻ vào địa chỉ của chương trình
    struct shared_data *shm_data = (struct shared_data*) shmat(shm_id, NULL, 0);
    if (shm_data == (void*) -1) {
        perror("Không thể gắn bộ nhớ chia sẻ");
        exit(1);
    }

    // Tạo hai tiến trình con
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("Lỗi khi tạo tiến trình con 1");
        exit(1);
    }

    if (pid1 == 0) {
        // Tiến trình con 1 xử lý tệp movie-100k_1.txt
        float total_rating_1;
        int count_1;
        compute_average("movie-100k_1.txt", &total_rating_1, &count_1);

        // Lưu kết quả vào bộ nhớ chia sẻ
        shm_data->total_rating_1 = total_rating_1;
        shm_data->count_1 = count_1;
        exit(0);  // Kết thúc tiến trình con 1
    }

    // Tạo tiến trình con thứ 2
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("Lỗi khi tạo tiến trình con 2");
        exit(1);
    }

    if (pid2 == 0) {
        // Tiến trình con 2 xử lý tệp movie-100k_2.txt
        float total_rating_2;
        int count_2;
        compute_average("movie-100k_2.txt", &total_rating_2, &count_2);

        // Lưu kết quả vào bộ nhớ chia sẻ
        shm_data->total_rating_2 = total_rating_2;
        shm_data->count_2 = count_2;
        exit(0);  // Kết thúc tiến trình con 2
    }

    // Chờ các tiến trình con hoàn thành
    waitpid(pid1, NULL, 0);  // Chờ tiến trình con 1
    waitpid(pid2, NULL, 0);  // Chờ tiến trình con 2

    // Tính điểm trung bình cho từng tệp
    float avg_rating_1 = (shm_data->count_1 == 0) ? 0.0 : shm_data->total_rating_1 / shm_data->count_1;
    float avg_rating_2 = (shm_data->count_2 == 0) ? 0.0 : shm_data->total_rating_2 / shm_data->count_2;

    printf("Điểm trung bình cho movie-100k_1.txt: %.2f\n", avg_rating_1);
    printf("Điểm trung bình cho movie-100k_2.txt: %.2f\n", avg_rating_2);

    // Tính điểm trung bình tổng thể
    int total_count = shm_data->count_1 + shm_data->count_2;
    float total_avg_rating = (total_count == 0) ? 0.0 : (shm_data->total_rating_1 + shm_data->total_rating_2) / total_count;
    printf("Điểm trung bình tổng thể: %.2f\n", total_avg_rating);

    // Giải phóng bộ nhớ chia sẻ
    shmdt(shm_data);
    shmctl(shm_id, IPC_RMID, NULL);  // Xóa bộ nhớ chia sẻ

    return 0;
}
