# PGZ-Disk-Reader
A very basic forensic tool with the following features.

Partition information:
- Display the number of partitions on the disk and for each partition display the start sector, size of partition and file system type.

FAT Volume information:
– Display the number of sectors per cluster, the size of the FAT area, the size of the Root Directory, and the sector address of Cluster #2.
- Display for the first deleted file on the volume’s root directory: the name and size of the file, and the number of the first cluster. Display the first 16 characters of the content of that file (assume it is a simple text file).
NTFS Volume information:
- Display bytes per sector and sectors per cluster for the NTFS volume
- Display ector address and type and length of the first two attributes in the $MFT record
