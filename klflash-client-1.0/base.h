
#ifndef __BASE_H
#define __BASE_H

#define PUSH_REGS \
	"pushl %%eax\n\t" \
	"pushl %%ebx\n\t" \
	"pushl %%ecx\n\t" \
	"pushl %%edx\n\t" \
	"pushl %%edi\n\t" \
	"pushl %%esi\n\t" \
	"pushl %%esp\n\t" \
	"pushl %%ebp\n\t"

#define POP_REGS \
	"popl %%ebp\n\t" \
	"popl %%esp\n\t" \
	"popl %%esi\n\t" \
	"popl %%edi\n\t" \
	"popl %%edx\n\t" \
	"popl %%ecx\n\t" \
	"popl %%ebx\n\t" \
	"popl %%eax\n\t"

#define MK_FP(seg, ofs) (((unsigned int)(seg)<<(4)) + (unsigned int)(ofs))
#endif
