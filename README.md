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

#### Laporan task 1 - Trabowo & Peddy Movie Night

#### Soal A - Ekstraksi File ZIP


![Hasil curl & unzip](/assets/task-1/task1-soalA.png "SS 1A")
![Hasil curl & unzip](/assets/task-1/task1-soalA2.png "SS 1A")

##### Penjelasan
Dalam soal ini, menggunakan 2 fork. Fork pertama untuk mendownload ```film.zip``` yang terdapat pada link gdrive. Pada fork, jika pid < 0 maka fork gagal membuat proses baru. Jika pid == 0 maka fork berhasil membuat proses child. proses child akan menjalankan
perintah download menggunakan ```downloadargv[6]``` yang berisi perintah ```curl -L -o film.zip https://drive.google.com/uc?export=download&id=1nP5kjCi9ReDk5ILgnM7UCnrQwFH67Z9B```. ```curl``` adalah command untuk mendownload data dari internet. ```-L``` artinya menyuruh ```curl``` mengikuti redirect kalau link diarahkan. ```-o film.zip``` untuk
menyimpan hasil download ke file lokal bernama ```film.zip```. Lalu execv digunakan untuk mengeksekusi argumen yang diberikan. Jika execv berhasil dijalankan, maka program apapun yang tertulis setelahnya tidak akan dijalankan. Jika tidak berhasil, maka ```perror``` akan dijalankan dan mengeluarkan pesan error serta program akan exit dengan status code 1. 

Pada kode, ```wait(NULL)``` digunakan untuk menunggu proses child hingga selesai. ```NULL``` artinya tidak memedulikan exit status dari proses child, hanya tunggu sampai selesai mendownload. Kemudian, fork kedua digunakan untuk mengunzip file ```film.zip```.

###### Kendala
Awalnya, saya tidak tahu kalau soalnya juga bermaksud untuk membuat program untuk mendownload film.zip dari link yang diberikan.

#### Soal B - Pemilihan Film Secara Acak

![Hasil pemilihan film](/assets/task-1/task1-soalB.png "SS 1B")

##### Penjelasan
Pada kode, ```DIR *folder=opendir("film")``` digunakan untuk membuka folder film dan ```*folder``` adalah pointer ke direktori filenya. 
Pada ```struct dirent *entri```, ```*entri``` adalah pointer ke struct dirent yang merupakan struct yang disediakan oleh Linux dan didefinisikan dalam library ```dirent.h```.
Lalu, ```listfilm[50]``` untuk menyimpan judul-judul film nantinya (terdapat 50 film). ```count``` untuk menghitung jumlah file yang ditemukan.

Menggunakan loop ```while((entri=readdir(folder))!=NULL)``` untuk membaca setiap entri dalam folder hingga akhir.
Lalu, jika filenya berakhiran dengan ekstensi ```.jpg``` maka akan masuk ke dalam array listfilm dan count pun bertambah. ```closedir(folder)``` untuk menutup folder.
Lalu, ```srand(time(NULL))``` akan digunakan untuk mengenerate angka random yang berbeda-beda setiap kali program dijalankan. ``` int index=rand()%count``` akan menghasilkan angka random dari 0 hingga count-1.
Lalu, printf akan mencetak judul film yang berbeda setiap kali program dijalankan. ```free(listfilm[i])``` untuk membebaskan memori.

###### Kendala
Tidak ada.

#### Soal C - Memilah Film Berdasarkan Genre

![Hasil pindah folder](/assets/task-1/task1-soalC.png "SS 1C")
![Hasil recap.txt](/assets/task-1/task1-soalC2.png "SS 1C")
![Hasil total.txt](/assets/task-1/task1-soalC3.png "SS 1C")

##### Penjelasan
Pertama, membuat mutex untuk digunakan nanti, buat variabel global, dan juga struct ```ThreadArgs```. void ```createFolderMk``` untuk membuat direktori baru, yaitu FilmAnimasi, FilmHorror, dan FilmDrama. Kemudian, void ```writeActivityLog``` untuk membuat log ```recap.txt```, menggunakan mutex sehingga hanya 1 thread dalam satu waktu yang bisa menulis ke ```recap.txt```, jika tidak menggunakan mutex bisa terjadi race condition. Lalu, menggunakan ```usleep(200)``` untuk memberi jeda thread bekerja agar tidak terjadi race condition.

Lalu, fungsi int ```compareFileName``` digunakan untuk membandingkan file berdasarkan nomor di awal file. Lalu, pada fungsi void ```listFiles``` akan membaca semua entri dalam folder film, menyimpan nama file dan path, dan menggunakan qsort untuk mengurutan file berdasarkan nomor awalnya. Lalu, pada fungsi void ```*moveFilesThread``` akan dijalankan oleh masing-masing thread, yaitu trabowo dan peddy. Lalu, akan dicek genre berdasarkan nama filenya, kemudian menentukan tujuan foldernya dan menjalankan fungsi ```writeActivityLog```. Tidak lupa untuk membebaskan memori dengan free.

Kemudian, fungsi void ```writeTotalLog``` untuk menulis ke total.txt jumlah film masing-masing genre dan juga top filmnya. Lalu, pada fungsi void ```sortFileGenre``` akan membuat folder FilmAnimasi, FilmDrama, dan FilmHorror. Kemudian, dipanggil fungsi ```listFiles``` untuk mendapatkan semua file yang sudah terurut. File akan dibagi 2 bagian, bagian trabowo (1-25) dan peddy (26-50), lalu buat 2 thread untuk memindahkan file secara pararel. Setelah selesai, memori files dan paths akan dibebaskan. Lalu, pada fungsi main tinggal memanggil fungsi ```sortFileGenre``` dan ```writeTotalLog```

##### Kendala
Kode saya direvisi karena hasil log dalam recap.txt tidak berurutan. Pada awalnya, membuat 3 thread untuk masing-masing genre, tapi sekarang saya ganti menjadi 2 thread untuk trabowo dan peddy.

#### Soal D - ZIP Folder Baru

![Hasil terminal](/assets/task-1/task1-soalD.png "SS 1D")
![Hasil zip folder](/assets/task-1/task1-soalD2.png "SS 1D")

##### Penjelasan
deklarasi array zip dan array folder. Kemudian, buat 3 fork untuk mengzip masing-masing genre. Jika pid < 0 artinya fork gagal, dan jika pid == 0, child proses akan menjalankan execv yang berisi argumen untuk mengzip folder. Jika execv tidak berhasil dijalankan, 
maka akan mengeluarkan pesan error dan exit(1). Kemudian, ```wait(NULL)``` untuk menunggu proses child selesai dijalankan. 

Setelah itu, buat 3 fork lagi untuk menjalankan perintah remove folder. Kemudian, buat ```wait(NULL)``` sebanyak 3 kali untuk menunggu execv berhasil dijalankan.

#### Kendala
Awalnya, saya tidak tahu bahwa setelah mengzip kembali folder, harus dilakukan juga penghapusan folder lama, karena tidak tertera secara eksplisit pada soal.


