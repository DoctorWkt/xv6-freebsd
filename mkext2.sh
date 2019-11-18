dd if=/dev/zero of=ext2.img bs=1M count=1
mkfs.ext2 -b 1024 -T small ext2.img
