
.PHONY: core

#OBJDIR  := objs/
OBJS    := #empty
OBJS    += $(OBJDIR)arith_utils.o
OBJS    += $(OBJDIR)openblas_residue.o
OBJS    += $(OBJDIR)basis.o
OBJS    += $(OBJDIR)dbg_print.o
OBJS    += $(OBJDIR)mpz_matrix.o
OBJS    += $(OBJDIR)reconstruct.o
OBJS    += $(OBJDIR)rns_matrix.o
OBJS    += $(OBJDIR)thresholds.o
OBJS    += $(OBJDIR)timer.o
OBJS    += $(OBJDIR)pl_matrix.o

core: $(OBJS)

$(OBJDIR)%.o : %.c
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm $(OBJS)
