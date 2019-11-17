dd if=/dev/zero of=ext2.img bs=10M count=1
mkfs.ext2 -b 1024 -T "EXT2_TEST" ext2.img
