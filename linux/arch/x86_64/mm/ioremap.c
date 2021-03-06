// $Id$
// $Locker$

// Author. Roar Thronæs.
// Modified Linux source file, 2010  
// Based on 2.6.16/17

/*
 * arch/x86_64/mm/ioremap.c
 *
 * Re-map IO memory to kernel address space so that we can access it.
 * This is needed for high PCI addresses that aren't mapped in the
 * 640k-1MB IO memory area on PC's
 *
 * (C) Copyright 1995 1996 Linus Torvalds
 */

#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/pgalloc.h>
#include <asm/fixmap.h>
/*
// skip this 16/17-part
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
*/
#include <linux/sched.h>
#include <asm/current.h>

// skip this 16/17-part
#define __force
#define __iomem

#include <asm/proto.h>

#define ISA_START_ADDRESS      0xa0000
#define ISA_END_ADDRESS                0x100000

static inline void remap_area_pte(pte_t * pte, unsigned long address, unsigned long size,
	unsigned long phys_addr, unsigned long flags)
{
	unsigned long end;
	unsigned long pfn;

	address &= ~PMD_MASK;
	end = address + size;
	if (end > PMD_SIZE)
		end = PMD_SIZE;
	if (address >= end)
		BUG();
	pfn = phys_addr >> PAGE_SHIFT;
	do {
		if (!pte_none(*pte)) {
			printk("remap_area_pte: page already exists\n");
			BUG();
		}
		set_pte(pte, pfn_pte(pfn, __pgprot(_PAGE_PRESENT | _PAGE_RW | 
					_PAGE_GLOBAL | _PAGE_DIRTY | _PAGE_ACCESSED | flags)));
		address += PAGE_SIZE;
		pfn++;
		pte++;
	} while (address && (address < end));
}

static inline int remap_area_pmd(pmd_t * pmd, unsigned long address, unsigned long size,
	unsigned long phys_addr, unsigned long flags)
{
	unsigned long end;

	address &= ~PUD_MASK;
	end = address + size;
	if (end > PUD_SIZE)
		end = PUD_SIZE;
	phys_addr -= address;
	if (address >= end)
		BUG();
	do {
		pte_t * pte = pte_alloc(&init_mm, pmd, address);
// skip this 16/17-part ..._kernel
		if (!pte)
			return -ENOMEM;
		remap_area_pte(pte, address, end - address, address + phys_addr, flags);
		address = (address + PMD_SIZE) & PMD_MASK;
		pmd++;
	} while (address && (address < end));
	return 0;
}

static inline int remap_area_pud(pud_t * pud, unsigned long address, unsigned long size,
	unsigned long phys_addr, unsigned long flags)
{
	unsigned long end;

	address &= ~PGDIR_MASK;
	end = address + size;
	if (end > PGDIR_SIZE)
		end = PGDIR_SIZE;
	phys_addr -= address;
	if (address >= end)
		BUG();
	do {
		pmd_t * pmd = pmd_alloc(&init_mm, pud, address);
		if (!pmd)
			return -ENOMEM;
		remap_area_pmd(pmd, address, end - address, address + phys_addr, flags);
		address = (address + PUD_SIZE) & PUD_MASK;
		pud++;
	} while (address && (address < end));
	return 0;
}

static int remap_area_pages(unsigned long address, unsigned long phys_addr,
				 unsigned long size, unsigned long flags)
{
	int error;
	pgd_t *pgd;
	unsigned long end = address + size;

	phys_addr -= address;
	pgd = pgd_offset_k(address);
	pgd_t *pgd2; // check TODO myhack
	struct task_struct * pcb = current;
	pgd2 = pgd_offset(pcb->mm, address);
	flush_cache_all();
	if (address >= end)
		BUG();
	// add this removed 16/17-part
	spin_lock(&init_mm.page_table_lock);
	do {
		pud_t *pud;
		pud = pud_alloc(&init_mm, pgd, address);
		*pgd2 = *pgd; // check TODO myhack
		error = -ENOMEM;
		if (!pud)
			break;
		if (remap_area_pud(pud, address, end - address,
					 phys_addr + address, flags))
			break;
		error = 0;
		address = (address + PGDIR_SIZE) & PGDIR_MASK;
		pgd++;
	} while (address && (address < end));
	// add this removed 16/17-part
	spin_unlock(&init_mm.page_table_lock);
	flush_tlb_all();
	return error;
}

/*
 * Fix up the linear direct mapping of the kernel to avoid cache attribute
 * conflicts.
 */
// skip this 16/17-part
static int
ioremap_change_attr(unsigned long phys_addr, unsigned long size,
					unsigned long flags)
{
	int err = 0;
	if (phys_addr + size - 1 < (end_pfn_map << PAGE_SHIFT)) {
		unsigned long npages = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
		unsigned long vaddr = (unsigned long) __va(phys_addr);

		/*
 		 * Must use a address here and not struct page because the phys addr
		 * can be a in hole between nodes and not have an memmap entry.
		 */
// skip this 16/17-part
/*
		err = change_page_attr_addr(vaddr,npages,__pgprot(__PAGE_KERNEL|flags));
		if (!err)
			global_flush_tlb();
*/
	}
	return err;
}

/*
 * Generic mapping function
 */

/*
 * Remap an arbitrary physical address space into the kernel virtual
 * address space. Needed when the kernel wants to access high addresses
 * directly.
 *
 * NOTE! We need to allow non-page-aligned mappings too: we will obviously
 * have to convert them into an offset in a page-aligned mapping, but the
 * caller shouldn't need to know that small detail.
 */
void __iomem * __ioremap(unsigned long phys_addr, unsigned long size, unsigned long flags)
{
	void * addr;
	struct vm_struct * area;
	unsigned long offset, last_addr;

	/* Don't allow wraparound or zero size */
	last_addr = phys_addr + size - 1;
	if (!size || last_addr < phys_addr)
		return NULL;

	/*
	 * Don't remap the low PCI/ISA area, it's always mapped..
	 */
	if (phys_addr >= ISA_START_ADDRESS && last_addr < ISA_END_ADDRESS)
		return (__force void __iomem *)phys_to_virt(phys_addr);

#ifdef CONFIG_FLATMEM
	/*
	 * Don't allow anybody to remap normal RAM that we're using..
	 */
	if (last_addr < virt_to_phys(high_memory)) {
		char *t_addr, *t_end;
 		struct page *page;

		t_addr = __va(phys_addr);
		t_end = t_addr + (size - 1);
	   
		for(page = virt_to_page(t_addr); page <= virt_to_page(t_end); page++)
			if(!PageReserved(page))
				return NULL;
	}
#endif

	/*
	 * Mappings have to be page-aligned
	 */
	offset = phys_addr & ~PAGE_MASK;
	phys_addr &= PAGE_MASK;
	size = PAGE_ALIGN(last_addr+1) - phys_addr;

	/*
	 * Ok, go for it..
	 */
	area = get_vm_area(size, VM_IOREMAP | (flags << 20));
	if (!area)
		return NULL;
	// skip this 16/17-part
	// area->phys_addr = phys_addr;
	addr = area->addr;
	if (remap_area_pages((unsigned long) addr, phys_addr, size, flags)) {
		vfree(addr);
		// skip this 16/17-part was:
		// remove_vm_area((void *)(PAGE_MASK & (unsigned long) addr));
		return NULL;
	}
	// skip this 16/17-part
	/*
	if (flags && ioremap_change_attr(phys_addr, size, flags) < 0) {
		area->flags &= 0xffffff;
		vunmap(addr);
		return NULL;
	}
	*/
	return (__force void __iomem *) (offset + (char *)addr);
}

/**
 * ioremap_nocache     -   map bus memory into CPU space
 * @offset:    bus address of the memory
 * @size:      size of the resource to map
 *
 * ioremap_nocache performs a platform specific sequence of operations to
 * make bus memory CPU accessible via the readb/readw/readl/writeb/
 * writew/writel functions and the other mmio helpers. The returned
 * address is not guaranteed to be usable directly as a virtual
 * address. 
 *
 * This version of ioremap ensures that the memory is marked uncachable
 * on the CPU as well as honouring existing caching rules from things like
 * the PCI bus. Note that there are other caches and buffers on many 
 * busses. In particular driver authors should read up on PCI writes
 *
 * It's useful if some control registers are in such an area and
 * write combining or read caching is not desirable:
 * 
 * Must be freed with iounmap.
 */

void __iomem *ioremap_nocache (unsigned long phys_addr, unsigned long size)
{
	return __ioremap(phys_addr, size, _PAGE_PCD);
}

/**
 * iounmap - Free a IO remapping
 * @addr: virtual address from ioremap_*
 *
 * Caller must ensure there is only one unmapping for the same pointer.
 */
// skip this 16/17-part: volatile
void iounmap(/*volatile*/ void __iomem *addr)
{
// skip this 16/17-part
	if (addr > high_memory)
		return vfree((void *) (PAGE_MASK & (unsigned long) addr));
}
