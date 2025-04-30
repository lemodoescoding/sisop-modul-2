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

### Laporan Task 2 - Organize and Analyze Anthony's Favorite Films

#### Soal A - One Click and Done!

##### Penjelasan

Pada fungsi `problem_a()`. Pertama, menggunakan fork() untuk membuat child process, lalu execvp() menjalankan perintah `wget` untuk mengunduh file netflixData.zip dari google drive. Lalu, program membuat folder baru bernama film menggunakan `mkdir -p film`. Setelah itu, file ZIP yang telah diunduh akan diekstrak ke dalam folder `film/` dengan perintah `unzip -o netflixData.zip -d film`. Terakhir, file ZIP yang sudah diekstrak akan dihapus menggunakan perintah `rm -f netflixData.zip`. Setiap proses berjalan satu per satu dengan wait() untuk memastikan setiap anak proses selesai sebelum melanjutkan ke proses berikutnya.

![Csv](/assets/task-2/soalA.png "SS hasil kode.")

##### Kendala
Tidak ada.

#### Soal B - Sorting Like a Pro

##### Penjelasan
Pada soal Sorting Like a Pro, program bertujuan untuk mengelompokkan film berdasarkan huruf pertama dari judul film dan tahun rilisnya. Fungsi `separator()` membaca per karakter dan membedakan mana koma yang sebagai pemisah kolom dan mana yang hanya bagian dari teks di dalam tanda kutip dengan variabel `flag` sebagai penanda sehingga setiap baris dapat dipisahkan menjadi array kolom. Fungsi `file_log()` digunakan untuk mencatat setiap aktivitas pengelompokan film ke dalam file `log.txt` dengan format waktu real-time [HH:MM:SS] dengan keterangan proses tersebut kategori "Abjad" atau "Tahun" dan nama film yang dikelompokkan.

Fungsi `judul()`. Pertama membuat folder `judul/` dengan perintah `mkdir`. Lalu membuka file CSV film/netflixData.csv. Lalu skip header dengan perintah `fgets` dan membaca baris per baris, kemudian memisahkan isi baris dengan memanggil fungsi `separator()`. Mengambil huruf pertama dari judul film, menentukan apakah huruf tersebut alfanumerik atau tidak menggunakan `isalnum()` dan menuliskan informasi `Judul - Tahun - Sutradara` ke dalam file teks yang sesuai. Setiap perulangan juga memanggil `file_log` untuk mencatat aktifitas ke dalam `log.txt`. Sedangkan, di fungsi `tahun()` memproses hal yang sama dengan fungsi `judul()` yang membedakan hanya membuat folder `tahun/` dan mengelompokkan film berdasarkan tahun rilis. Agar kedua pengelompokan ini terjadi secara paralel dengan cara menggunakan 2 child proses di `main()`, yaitu pertama membuat child process untuk memanggil fungsi `judul()` dan parent process melakukan wait(). Lalu membuat child process kedua untuk memanggil `tahun()` dan juga dilanjutkan dengan `wait()` sehingga kedua pengelompokan bisa berjalan bersamaan.

![Kelompok](/assets/task-2/soalB-kelompok.png "SS hasil kode.")
![Log](/assets/task-2/soalB-log.png "SS hasil kode.")

##### Kendala
Saat mengelompokkan kolom sesuai header karena ada satu kolom terdapat beberapa data sehingga tidak bisa dipisahkan dengan menggunakan `strtok`

#### Soal C - The Ultimate Movie Report

##### Penjelasan
 `struct Report` berisi `negara` untuk menyimpan nama negara, serta `sebelum` dan `setelah` untuk menghitung berapa banyak film dari negara tersebut yang dirilis sebelum dan sesudah tahun 2000. Lalu fungsi `negara()` membuka file CSV dan dibaca per baris dan setiap baris dipisahkan kolomnya menggunakan fungsi `separator()`, diambil data negara dari kolom ketiga dan tahun rilis dari kolom keempat. Lalu dilakukan perulangan dan membandingkan `statistik[i].negara` dengan `negara` jika tidak ada maka `negara` tersebut akan dicopy. Jika `tahun` < 200 maka `sebelum` akan diinisialisai 1 dan `setelah` 0 dan `jumlah_negara` ditambah 1 dan sebaliknya. Jika sudah pernah dicopy maka akan ditambaha sesuai tahunnya dan di break.	

 Setelah semua baris diproses, fungsi `negara()` akan mengembalikan NULL. Setelah thread selesai melakukan tugasnya dan selesai bergabung (pthread_join). Program membuat file laporan dengan nama report_ddmmyyyy.txt di fungsi `laporan()` di mana tanggalnya diambil dari waktu saat ini menggunakan `strftime()`. Lalu diisi dengan daftar negara yang sudah diproses sebelumnya, diawali dengan nomor urut, nama negara, jumlah film sebelum 2000, dan jumlah film setelah 2000. Lalu menambahkan antarmuka terminal di fungsi main.

![UI & Report](/assets/task-2/soalC.png "SS hasil kode.")

##### Kendala
Tidak ada.
