# PGZ-Disk-Reader
A very basic forensic tool. This tool is for very specific use, i.e. it assumes that the first partition of the disk it reads is FAT-16 always and that the disk contains a deleted '.txt' file (analysis of deleted content is not flexible). The sample image disk alongside the reader (*Sample_1.dd*) is the disk against which the program was used --and developed for.

### Partition information:
- Display the number of partitions on the disk and for each partition display the start sector, size of partition and file system type.

#### *N.B.: Partition information code is heavily inspired from the code of Donal Heffernan Dr. (2016). It served as a basis for the following sections as well.*

### FAT Volume information:
- Display the number of sectors per cluster, the size of the FAT area, the size of the Root Directory, and the sector address of Cluster #2.
- Display for the first deleted file on the volume’s root directory: the name and size of the file, and the number of the first cluster. Display the first 16 characters of the content of that file (assume it is a simple text file).

### NTFS Volume information:
- Display bytes per sector and sectors per cluster for the NTFS volume
- Display sector address and type and length of the first two attributes in the $MFT record
