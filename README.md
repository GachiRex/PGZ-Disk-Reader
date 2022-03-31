# PGZ-Disk-Reader
A very basic forensic tool with the following features. Please note that this tool is for very specific use, i.e. it assumes that the disk it reads is FAT-16 only and contain a deleted '.txt' file (analysis of deleted content is not flexible).

Partition information:
- Display the number of partitions on the disk and for each partition display the start sector, size of partition and file system type.

N.B.: It should be noted that the partition information code is heavily inspired from the code of Donal Heffernan Dr. from the University of Limerick. It served as a basis for the following sections.

FAT Volume information:
- Display the number of sectors per cluster, the size of the FAT area, the size of the Root Directory, and the sector address of Cluster #2.
- Display for the first deleted file on the volume’s root directory: the name and size of the file, and the number of the first cluster. Display the first 16 characters of the content of that file (assume it is a simple text file).

NTFS Volume information:
- Display bytes per sector and sectors per cluster for the NTFS volume
- Display ector address and type and length of the first two attributes in the $MFT record
