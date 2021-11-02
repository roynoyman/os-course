//
// Created by Roy Noyman on 28/10/2021.
//

#include "os.h"

int VPN_PARTS_NUMBER = 5;
int MAX_MOVED_BITS = 36;


int current_pte_addr(uint64_t vpn, int i) {
    int curr_pte_part = vpn >> (MAX_MOVED_BITS - 9 * i);
    curr_pte_part = curr_pte_part & 0x1ff;
    return curr_pte_part;
}

int check_valid_bit(uint64_t pt_entry) {
    return pt_entry & 1;
}

uint64_t find_current_pte(uint64_t *page_table, uint64_t vpn, int vpn_part) {
    int pte_i = current_pte_addr(vpn, vpn_part);
    uint64_t curr_pte = page_table[pte_i];
    return curr_pte;
}

void store_new_ppn(uint64_t *page_table, uint64_t vpn, uint64_t ppn) {
    int i = 0;
    int pte_i;
    uint64_t ppn_to_store;
    if (ppn == NO_MAPPING) {
        ppn_to_store = 0;
    } else {
        ppn_to_store = (ppn << 12) + 1;
    }
    uint64_t correct_pte_addr;
    for (i = 0; i < VPN_PARTS_NUMBER - 1; i++) {
        uint64_t curr_pte = find_current_pte(page_table, vpn, i);
        if (check_valid_bit(curr_pte) == 0) {
            pte_i = current_pte_addr(vpn, i);
            page_table[pte_i] = (alloc_page_frame() << 12) + 1;
            page_table = phys_to_virt(page_table[pte_i] - 1);
        } else {
            page_table = phys_to_virt(curr_pte - 1);
        }
    }
    correct_pte_addr = vpn & 0x1ff;
    page_table[correct_pte_addr] = ppn_to_store;
}


void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    uint64_t phys_pt = pt << 12;
    uint64_t *page_table = phys_to_virt(phys_pt);
    uint64_t is_mapped = page_table_query(pt, vpn);
    if (ppn == NO_MAPPING && is_mapped == NO_MAPPING) {
        return;
    } else { //remove last part of vpn in pt
        store_new_ppn(page_table, vpn, ppn);
    }
}


uint64_t page_table_query_rec(uint64_t *page_table, uint64_t vpn, int vpn_part) {
    uint64_t curr_pte = find_current_pte(page_table, vpn, vpn_part);
    if (check_valid_bit(curr_pte) == 0) {
        return NO_MAPPING;
    }
    if (vpn_part == VPN_PARTS_NUMBER - 1) {
        return curr_pte >> 12;
    }
    page_table = phys_to_virt(curr_pte - 1);
    return page_table_query_rec(page_table, vpn, vpn_part + 1);
}


uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t phys_pt = pt << 12;
    uint64_t *page_table = phys_to_virt(phys_pt);
    uint64_t ppn = page_table_query_rec(page_table, vpn, 0);
    return ppn;
}
