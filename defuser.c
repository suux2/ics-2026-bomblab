#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    size_t count = 1;
    uint32_t lo = (uint32_t)challenge;
    uint32_t hi = (uint32_t)(challenge >> 32);
    uint32_t a = (UINT32_C(5) * lo + hi) ^ UINT32_C(0x7bf3);
    uint32_t b = ((uint32_t)(challenge >> 16) + UINT32_C(0xc54a)) ^ hi;
    uint32_t d = UINT32_C(5) * (b - lo) + UINT32_C(0x30ee);
    values[0] = d ^ a;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    values[0] = (uint32_t)((challenge >> 2) & 0x7f);
    for (uint32_t i = 1; i < 6; i++) {
        uint32_t t = (uint32_t)((challenge >> (23 + 5 * i)) & 0x3f) ^ i;
        values[i] = t + 4 * values[i - 1];
    }    // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    uint32_t x = (uint32_t)(challenge & 7);
    uint32_t t = (uint32_t)((challenge >> 32) ^ (challenge >> 3));
    uint32_t y = 0;
    switch (x) {
    case 0: y = (t ^ UINT32_C(0x7b4b)) + (t >> 1); break;
    case 1: y = UINT32_C(5) * t + UINT32_C(0x3b42); break;
    case 2: y = (UINT32_C(4) * t) ^ UINT32_C(0x31e4); break;
    case 3: y = UINT32_C(5) * t + UINT32_C(0x1dd3a); break;
    case 4: y = (UINT32_C(4) * t) ^ UINT32_C(0x6320); break;
    case 5: y = UINT32_C(5) * t + UINT32_C(0x26868); break;
    case 6: y = (t ^ UINT32_C(0xdad)) + (t >> 2); break;
    case 7: y = UINT32_C(5) * t + UINT32_C(0x3905); break;
    default: y = 0; break;
    }
    values[0] = x;
    values[1] = y;
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t a, uint32_t lo, uint32_t hi, uint32_t k) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    uint32_t t, r;
    if (mid == a) return (mid + UINT32_C(0xa4)) ^ k;
    t = mid + k;
    if (a < mid) {
        r = p4_walk(a, lo, mid - 1, k);
        return ((UINT32_C(3) * r) ^ t) ^ UINT32_C(1);
    }
    r = p4_walk(a, mid + 1, hi, k);
    return ((UINT32_C(3) * r) ^ t) ^ UINT32_C(2);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t a = (uint32_t)(challenge & 0x7f);
    uint32_t k = (uint32_t)((challenge >> 7) ^ (challenge >> 32));
    values[0] = p4_walk(a, 0, 127, k);
    return write_answer(out, cap, values, count);
}

struct p5_node { uint32_t a, b; int next; };

static const struct p5_node p5_nodes[12] = {
    {0xc98f, 0x538, -1}, {0xd15f, 0x756, 2},  {0x0c0f, 0x95f, 6},
    {0x301b, 0x076, 11}, {0x1169, 0x30b, 3},  {0xacc6, 0xb2b, 10},
    {0xa648, 0x259, 5},  {0x0dc1, 0x80d, 8},  {0xbb28, 0x167, 9},
    {0x0a9e, 0xa68, 0},  {0x30f7, 0x45a, 4},  {0xe508, 0x635, 7},
};
static const int p5_heads[8] = {1, 2, 6, 5, 10, 4, 3, 11};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    int cur = p5_heads[challenge & 7];
    uint32_t n = (uint32_t)((challenge >> 3) & 3);
    uint32_t acc = UINT32_C(3) *
        ((uint32_t)((challenge >> 6) ^ (challenge >> 32)) ^ UINT32_C(0xf63)) +
        p5_nodes[cur].a;
    uint32_t last = p5_nodes[cur].b;
    acc ^= last;
    for (uint32_t i = 0; i < n; i++) {
        cur = p5_nodes[cur].next;
        acc = UINT32_C(3) * acc + p5_nodes[cur].a;
        last = p5_nodes[cur].b;
        acc ^= last;
    }
    values[0] = acc;
    values[1] = last;
    return write_answer(out, cap, values, count);
}

static uint32_t p6_op(int k, uint32_t x) {
    switch (k) {
    case 0: return (x ^ (x >> 5)) + UINT32_C(0xfc8e);
    case 1: return (x >> 2) | (x << 30);
    case 2: return (x ^ UINT32_C(0xc948)) + (x << 7);
    case 3: return (x + UINT32_C(0xe45e)) ^ (x >> 1);
    case 4: return (UINT32_C(2) * x + UINT32_C(0x6b8a)) ^ (UINT32_C(5) * x);
    default: return ((x << 1) | (x >> 31)) ^ UINT32_C(0x12a5);
    }
}
static const int p6_ops[8] = {4, 2, 5, 0, 3, 5, 4, 1};

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t t = (uint32_t)((challenge >> 6) ^ (challenge >> 32));
    uint32_t r1 = p6_op(p6_ops[challenge & 7], t) ^ UINT32_C(0x1d9e);
    uint32_t r2 = p6_op(p6_ops[(challenge >> 3) & 7], r1);
    values[0] = r2 + UINT32_C(0xb363);
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    return 1;
}
