ALL:
	gcc -O2 -o Get.cgi Get.c
	gcc -O2 -o Post.cgi Post.c
clean:
	rm *.cgi
