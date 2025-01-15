#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <bit>
#include <emmintrin.h>
#include <random>

template<typename K, typename V>
class CSwissMap {
    const int kWidth = 16;
    enum class Ctrl : int8_t {
        kEmpty = -128,   // 0b10000000
        kDeleted = -2,   // 0b11111110
        kSentinel = -1,  // 0b11111111
    };
    struct Slot {
        K k;
        V v;
    };

    Ctrl* m_ctrl;               // 控制字节数组
    Slot* m_slot;               // 槽数组
    size_t m_cap = 0;           // 控制字节容量
    size_t m_num = 0;           // 控制字节使用量
    size_t m_growthInfo = 0;    // 在rehash前，可以存储的数量，变量名遵循absl中的命名
    size_t m_salt = 0;          // hash 盐

public:
    CSwissMap() {
        // 使用random_device生成一个随机数种子，
        // 再梅森旋转算法生成一个64位的随机数作为hash盐
        std::random_device rd;
        std::mt19937_64 gen(rd());
        m_salt = gen();
        init(0);
    }

    CSwissMap(const CSwissMap&) = delete;
    CSwissMap(const CSwissMap&&) = delete;
    CSwissMap& operator=(const CSwissMap&) = delete;
    CSwissMap& operator=(const CSwissMap&&) = delete;

    virtual ~CSwissMap() {
        delete[] m_ctrl;
        m_cap = 0;
        m_num = 0;
        m_growthInfo = 0;
        m_salt = 0;
    }

    V* operator[](K key) {
        return find(key, false);
    }

    void insert(const K& key, const V& v) {
        V* x = find(key, true);
        new(x) V(v);
    }

    bool erase(const K& key) {
        uint64_t index = 0;
        int8_t h2 = 0;
        if (findIndex(key, index, h2)) {
            // 设置删除标志
            setCtrl(index, (int8_t)Ctrl::kDeleted);
            // 销毁Slot
            destorySlot(m_slot[index]);
            // 已使用的控制字节数量减1
            m_num--;
            // 可存储槽数量加1
            m_growthInfo++;
            return true;
        }
        return false;
    }

private:
    constexpr size_t align(size_t x, size_t a) {
        return (x + a - 1) & (~(a - 1));
    }

    void init(size_t n) {
        // 控制字节总大小需要加上结束标志（1个字节）与克隆大小（15个字节）
        size_t ctrlSize = sizeof(Ctrl) * (n + kWidth);
        // 按16字节对齐
        size_t alignCtrlSize = align(ctrlSize, 16);
        // 槽大小
        size_t slotSize = sizeof(Slot) * n;
        // 总大小
        size_t size = alignCtrlSize + align(slotSize, 16);
        char* p = new char[size];
        m_ctrl = (Ctrl*)p;
        m_slot = (Slot*)(p + alignCtrlSize);
        // 将所有控制字节设置为空
        memset(m_ctrl, (int)Ctrl::kEmpty, ctrlSize);
        // 设置控制字节结束标志
        m_ctrl[n] = Ctrl::kSentinel;
        m_cap = n;
        // 根据控制字节容量以及使用情况设置增长信息
        m_growthInfo = (n - n / 8) - m_num;
    }

    void rehash(size_t n) {
        // 先保存之前的控制字节、槽以及容量信息
        auto ctrl = m_ctrl;
        auto slot = m_slot;
        auto cap = m_cap;
        // 使用新的容量分配内存
        init(n);
        // 重置已经使用的数量
        m_num = 0;
        // 将之前已经存在的数量，按新的内存重新分配
        for (auto i = 0; i < cap; ++i) {
            // 只处理有效控制字节
            if (int(ctrl[i]) >= 0) {
                Slot& s = slot[i];
                V* x = find(s.k, true);
                // 使用指定内存调用V的构造函数
                new(x) V(s.v);
                // 销毁原来的Slot
                destorySlot(s);
            }
        }
        // 释放原来的内存
        delete[] ctrl;
    }

    void destorySlot(Slot& slot) {
        // 由于Slot所在内存是与ctrl一起分配的，
        // 所以不能单独释放内存，这里只能调用析构函数
        // 调用Key的析构函数
        slot.k.~K();
        // 调用Value的析构函数
        slot.v.~V();
    }

    bool IsValidCapacity(size_t n) {
        return ((n + 1) & n) == 0 && n > 0;
    }

    // 下一次分配控制字节内存所需要的容量
    // 控制字节的容量始终为2的N次方减1
    size_t nextCap() {
        assert(IsValidCapacity((m_cap * 2) + 1));
        return (m_cap * 2) + 1;
    }

    // 获取槽的索引
    uint64_t getSlotIndex(uint64_t index, uint16_t mask) {
        // 根据H1计算的索引index加上匹配的mask索引
        // 为了避免越界，需要限制在m_cap范围内
        // 由于m_cap的二进制位全部是1，所以直接位与即可
        assert(m_cap == 0 || IsValidCapacity(m_cap));
        return (index + std::countr_zero(mask)) & m_cap;
    }

    // 根据key查找槽的索引
    // 成功找到返回true，retIndex为该key所对应的槽索引
    // 未找到返回false，retIndex为该key所对应的空槽索引
    bool findIndex(const K& key, uint64_t& retIndex, int8_t& h2) {
        std::hash<K> h;
        // 计算key的hash值
        uint64_t k = h(key);
        // 给hash值加盐
        k ^= m_salt;
        h2 = H2(k);
        // 一次性填充16字节的特征值h2
        __m128i match = _mm_set1_epi8(static_cast<char>(h2));
        uint64_t h1 = H1(k);
        // 根据h1计算控制字节的索引
        auto index = h1 & m_cap;
        while (true) {
            // 从index开始的位置，一次性读取16字节数据
            __m128i ctrl = _mm_loadu_si128((const __m128i*)(&m_ctrl[index]));
            // 与填充的特征值进行“相等”比较，即：match == ctrl
            auto x = _mm_cmpeq_epi8(match, ctrl);
            // 将比较的结果进行压缩
            auto mask = static_cast<uint16_t>(_mm_movemask_epi8(x));
            while (mask != 0) { // 压缩的结果不为0，则表示有匹配的特征值
                // 计算槽相对m_ctrl的索引
                auto slotIndex = getSlotIndex(index, mask);
                assert((uint64_t)m_ctrl[slotIndex] == h2);
                // 如果是指定的Key则返回
                if (m_slot[slotIndex].k == key) {
                    retIndex = slotIndex;
                    return true;
                }
                // 由于处理时是计算的尾部有多少个0，
                // 所以之前处理的是低位的第一个1，现在将其置为0
                mask &= mask - 1;
            }
            // 压缩结果为0，则没有匹配的特征值，检查空槽
            __m128i matchEmpty = _mm_set1_epi8(static_cast<char>(Ctrl::kEmpty));
            x = _mm_cmpeq_epi8(matchEmpty, ctrl);
            mask = static_cast<uint16_t>(_mm_movemask_epi8(x));
            if (mask != 0) {
                // 计算槽相对m_ctrl的索引
                retIndex = getSlotIndex(index, mask);
                return false;
            }
            // 当前group没有找到，定位到下一group，继续查找
            index += kWidth;
            // 索引不能越界
            index &= m_cap;
        }
    }

    V* find(const K& key, bool isAdd) {
    refind:
        uint64_t index = 0;
        int8_t h2 = 0;
        if (findIndex(key, index, h2)) {
            // 如果成功找到，返回Value的地址
            return &m_slot[index].v;
        }
        // 没有找到，需要添加
        if (isAdd) {
            // 一次性读取16个字节
            __m128i ctrl = _mm_loadu_si128((const __m128i*)(&m_ctrl[index]));
            // 填充16个字节的结束标志
            __m128i specail = _mm_set1_epi8(static_cast<char>(Ctrl::kSentinel));
            // 与specail进行“大于”比较，即：specail > ctrl，筛选出所有空及已删除的标志
            // specail中填写的是-1，所以大于空（-128）以及删除标志（-2），
            // 但不大于有效控制字节，因为有效控制字节是非负数
            auto x = _mm_cmpgt_epi8(specail, ctrl);
            uint16_t mask = static_cast<uint16_t>(_mm_movemask_epi8(x));
            if (mask != 0) {
                // 如果还可以存储，直接存储
                if (m_growthInfo > 0) {
                    // 这里需要根据mask重新计算索引
                    index = getSlotIndex(index, mask);
                    assert(m_ctrl[index] == Ctrl::kEmpty || m_ctrl[index] == Ctrl::kDeleted);
                    // 在控制字节中设置特征值h2
                    setCtrl(index, h2);
                    // 在设置对应槽的Key
                    new(&m_slot[index].k) K(key);
                    // 增加控制字节使用数
                    m_num++;
                    // 减少可存储数
                    m_growthInfo--;
                    return &m_slot[index].v;
                }
            }
            // 不能再存储了，需要rehash重新分配空间了
            rehash(nextCap());
            // 重新分配了空间，需要重新查找
            goto refind;
        }
        // 只查找，没找到
        return nullptr;
    }

    void setCtrl(uint64_t index, int8_t h) {
        m_ctrl[index] = (Ctrl)h;
        if (index < kWidth - 1) {
            // 如果index小于克隆字节大小，则需要同时设置克隆区的控制字节
            m_ctrl[index + m_cap + 1] = (Ctrl)h;
        }
    }

    uint64_t H1(uint64_t hash) {
        // 取hash值的高57位，并加盐，
        // 盐是根据当前控制字节的首地址右移12位，12位刚好是一个内存页大小
        return (hash >> 7) ^ ((uintptr_t)(m_ctrl) >> 12);
    }

    int8_t H2(uint64_t hash) {
        // 取hash值的低7位
        return (int8_t)(hash & 0x7F);
    }
};
