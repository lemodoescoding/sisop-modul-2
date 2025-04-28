[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/9LcL5VTQ)
| NRP | Name |
| :--------: | :------------: |
| 5025241054 | Andie Azril Alfrianto |
| 5025241060 | Christina Tan |
| 5025241061 | Ahmad Satrio Arrohman |

# Praktikum Modul 2 _(Module 2 Lab Work)_

</div>

### Daftar Soal _(Task List)_

- [Task 1 - Trabowo & Peddy Movie Night](/task-1/)

- [Task 2 - Organize and Analyze Anthony's Favorite Films](/task-2/)

- [Task 3 - Cella’s Manhwa](/task-3/)

- [Task 4 - Pipip's Load Balancer](/task-4/)

### Laporan Resmi Praktikum Modul 2 _(Module 2 Lab Work Report)_

Tulis laporan resmi di sini!

_Write your lab work report here!_

#### Laporan Task 4 - Pipip's Load Balancer

Compile dengan

```bash
gcc client.c -o client
gcc loadbalancer.c -o loadbalancer
gcc worker.c -o worker -pthread
```

```c
void report_and_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}
```

Terdapat fungsi report_and_error yang digunakan untuk mempersingkat penulisan perror yang digunakan
statement yang ada dalam fungsi report_and_error. Fungsi report_and_error akan dipanggil dalam kasus ketika terjadi kesalahan dalam pemanggilan
fungsi lain seperti `fopen`,`curl_easy_init` dsb. Fungsi report_and_error memiliki satu parameter yang berisi custom message error
dan di dalam fungsi report_and_error terdapat pemanggilan fungsi `perror` dengan argumen berisi custom message error tadi. Fungsi `perror`
akan memberikan suatu buffer ke `stderr` dengan tambahan keterangan error otomatis saat perror ini dipanggil.
Kemudian jika fungsi report_and_error maka program akan langsung berhenti karena adanya pemanggilan fungsi `exit` yang memaksa
program untuk diberhentikan eksekusinya.

#### Soal A -

```c

```

##### Penjelasan

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal A

### Soal B -

```c

```

##### Penjelasan

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal B

#### Soal C -

```c

```

##### Penjelasan

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal C

#### Soal D -

```c

```

##### Penjelasan

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal D
