#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest_sub(unsigned int start, unsigned int end) {
    unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
    for (i = start; i <= end; i += 0x1000) {
        p = (unsigned int *) (i + 0xffc);
        old = *p;           // 弄る前の値をoldに保存
        *p = pat0;          // 試しに書いてみる
        *p ^= 0xffffffff;   // それを反転する
        if (*p != pat1) {  // 反転結果になったか
            not_memory:
            *p = old;
            break;
        }
        *p ^= 0xffffffff;
        if (*p != pat0) {
            goto not_memory;
        }
        *p = old;       // いじった値をもとに戻す
    }
    return i;
}

unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    // 386か486以降なのかを確認
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; // AC-bit = 1
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0) {
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT; // AC-bit = 0;
    io_store_eflags(eflg);

    if (flg486) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE; // キャッシュ禁止
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flg486) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; // キャッシュ許可
        store_cr0(cr0);
    }

    return i;
}

void memman_init(struct MEMMAN *man) {
    man->frees = 0;
    man->maxfrees = 0;
    man->lostsize = 0;
    man->losts = 0;
    return;
}

unsigned int memman_total(struct MEMMAN *man) {
    unsigned int i, t = 0;
    for (i = 0; i < man->frees; i++) {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size) {
    unsigned int i, a;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].size >= size) {
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0) {
                // free[i]がなくなったので前へ詰める
                man->frees--;
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i+1];
                }
            }
            return a;
        }
    }
    return 0; // 秋なし
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size) {
    int i, j;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].addr > addr) {
            break;
        }
    }
    if (i > 0) {
        if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
            // 前の空き領域にまとめられる
            man->free[i - 1].size += size;
            if (i < man->frees) {
                if (addr + size == man->free[i].addr) {
                    // 後ろとまとめられる
                    man->free[i-1].size += man->free[i].size;
                    man->frees--;
                    for (; i < man->frees; i++) {
                        man->free[i] = man->free[i + 1];
                    }
                }
            }
            return 0;
        }
    }
    // 前とまとめられない場合
    if (i < man->frees) {
        if (addr + size == man->free[i].addr) {
            // 後ろとまとめられる
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0;
        }
    }
    // 前にも後ろにもまとめられない
    if (man->frees < MEMMAN_FREES) {
        // free[i]より後ろを後ろへずらして隙間を作る
        for (j = man->frees; j > i; j--) {
            man->free[j] = man->free[j - 1];
        }
        man->frees++;
        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees; // 最大値を更新
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    // 後ろにずらせなかった
    man->losts++;
    man->lostsize += size;
    return -1;
}