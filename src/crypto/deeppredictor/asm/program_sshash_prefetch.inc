	and rbx, DEEPPREDICTOR_CACHE_MASK
	shl rbx, 6
	add rbx, rdi
	prefetchnta byte ptr [rbx]