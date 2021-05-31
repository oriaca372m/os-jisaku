extern "C" void KernelMain() {
	while (true) {
		__asm("hlt");
	}
}
