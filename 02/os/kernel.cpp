#include <stdint.h>
#include <stddef.h>

#include "stivale2.h"

#define COM1 0x3f8 // COM1

void* memset(void* ptr, uint16_t value, size_t num)
{
    if (ptr == NULL) return NULL;

    char* tempPtr = (char*)ptr;
    while(num--) *tempPtr++ = value;

    return ptr;
}

void* memcpy (void* destination, const void* source, size_t num)
{
    if (destination == NULL || source == NULL) return NULL;  

    char *tempDest= (char*)destination;  
    char *tempSrc = (char*)source;  
  
    while (num-- > 0) *tempDest++ = *tempSrc++;  

    return destination;      
}

// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an uninitialised array in .bss.
static uint8_t stack[4096];
 
// stivale2 uses a linked list of tags for both communicating TO the
// bootloader, or receiving info FROM it. More information about these tags
// is found in the stivale2 specification.
 
// We are now going to define a framebuffer header tag, which is mandatory when
// using the stivale2 terminal.
// This tag tells the bootloader that we want a graphical framebuffer instead
// of a CGA-compatible text mode. Omitting this tag will make the bootloader
// default to text mode, if available.
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        .next = 0
    },
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 32
};
 
// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = 0,
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (uintptr_t)stack + sizeof(stack),
    // Bit 1, if set, causes the bootloader to return to us pointers in the
    // higher half, which we likely want.
    .flags = (1 << 1),
    // This header structure is the root of the linked list of header tags and
    // points to the first one in the linked list.
    .tags = (uintptr_t)&framebuffer_hdr_tag
};
 
// We will now write a helper function which will allow us to scan for tags
// that we want FROM the bootloader (structure tags).
void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
    struct stivale2_tag *current_tag = (stivale2_tag *)stivale2_struct->tags;
    for (;;) {
        // If the tag pointer is NULL (end of linked list), we did not find
        // the tag. Return NULL to signal this.
        if (current_tag == NULL) {
            return NULL;
        }
 
        // Check whether the identifier matches. If it does, return a pointer
        // to the matching tag.
        if (current_tag->identifier == id) {
            return current_tag;
        }
 
        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (stivale2_tag *)current_tag->next;
    }
}
 
static inline void outportb(uint16_t port, uint8_t data) {
    asm("outb %1, %0" : : "dN" (port), "a" (data));
}

static inline uint8_t inportb(uint16_t port) {
    uint8_t r;
    asm("inb %1, %0" : "=a" (r) : "dN" (port));
    return r;
}

class SerialInterface
{
public:
    SerialInterface();

    int IsTransmitEmpty();
    void SerialWriteChar(const char a);
    void SerialWriteInt(uint16_t value, uint16_t base);
    void SerialWriteStr(const char* msg);
};

SerialInterface::SerialInterface() {
    // Init serial logging
    outportb(COM1 + 1, 0x00);   // Disable all interrupts
    outportb(COM1 + 3, 0x80);   // Enable DLAB (set baud rate divisor)
    outportb(COM1 + 0, 0x03);   // Set divisor to 3 (lo byte) 38400 baud
    outportb(COM1 + 1, 0x00);   //                  (hi byte)
    outportb(COM1 + 3, 0x03);   // 8 bits, no parity, one stop bit
    outportb(COM1 + 2, 0xC7);   // Enable FIFO, clear them, with 14-byte threshold
    outportb(COM1 + 4, 0x0B);   // IRQs enabled, RTS/DSR set
    outportb(COM1 + 4, 0x1E);   // Set in loopback mode, test the serial chip
    outportb(COM1 + 0, 0xAE);   // Test serial chip (send byte 0xAE and check if serial returns same byte)
    
    // Check if serial is faulty (i.e: not same byte as sent)
    if(inportb(COM1 + 0) != 0xAE) { /* Stop?.. */ }
    
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outportb(COM1 + 4, 0x0F);
}

int SerialInterface::IsTransmitEmpty() 
{ 
    return inportb(COM1 + 5) & 0x20;
}

void SerialInterface::SerialWriteChar(const char a)
{
    while (IsTransmitEmpty() == 0);

    outportb(COM1, a); 
}

void SerialInterface::SerialWriteInt(uint16_t value, uint16_t base = 10)
{
    // https://www.geeksforgeeks.org/implement-itoa/
    char tmpb[21], *tmp = &tmpb[0];
    for (; value > 0; value /= base)
        *(++tmp) = "0123456789abcdef"[value % base];    
    while (*tmp != '\0') 
        SerialWriteChar(*(tmp--));
}

void SerialInterface::SerialWriteStr(const char* msg)
{
    for (size_t i = 0; 1; i++) {
        if ((char)msg[i] == '\0')
        {
            return;
        }

        SerialWriteChar((char)msg[i]);
    }
}

class ErebOS
{
public:
    ErebOS(stivale2_struct* stivale2_struct);

    inline uint32_t ColorFromChannels(uint8_t r, uint8_t g, uint8_t b) 
    {
        return ((r << 24) + (g << 16) + (b << 8) + (uint8_t)0x00);
    }

    inline void DrawPixel(uint32_t color, uint16_t x, uint16_t y)
    {
        m_Framebuffer[y * m_FramebufferWidth + x] = color;
    }

    inline void DrawRect(uint32_t color, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height) 
    {
        for (size_t y = posY; y < posY + height; y++)
        {
            for (size_t x = posX; x < posX + width; x++)
            {
                DrawPixel(color, x, y);
            }
        }
    }

private:
    SerialInterface m_Serial;
    uint32_t* m_Framebuffer;
    uint16_t m_FramebufferWidth;
    uint16_t m_FramebufferHeight;
};

ErebOS::ErebOS(stivale2_struct* stivale2_struct)
{
    // Get framebuffer info from bios
    stivale2_struct_tag_framebuffer *fb_str_tag;
    fb_str_tag = (stivale2_struct_tag_framebuffer*) stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);
    m_Framebuffer = (uint32_t *)fb_str_tag->framebuffer_addr;
    m_FramebufferWidth = (uint16_t)fb_str_tag->framebuffer_width;
    m_FramebufferHeight = (uint16_t)fb_str_tag->framebuffer_height;

    m_Serial.SerialWriteStr("Framebuffer info: \n\r");
    m_Serial.SerialWriteStr(" > Framebuffer width: ");
    m_Serial.SerialWriteInt(m_FramebufferWidth);
    m_Serial.SerialWriteStr("\n\r");
    m_Serial.SerialWriteStr(" > Framebuffer height: ");
    m_Serial.SerialWriteInt(m_FramebufferHeight);
    m_Serial.SerialWriteStr("\n\r");

    // Draw to screen
    for (size_t x = 0; x < m_FramebufferWidth; x++)
    {
        for (size_t y = 0; y < m_FramebufferHeight; y++)
        {
            if (x % 2 == 1) DrawPixel(ColorFromChannels((x*255)/m_FramebufferWidth, (y*255)/m_FramebufferHeight, (x*255)/m_FramebufferWidth), x, y);
        }
    }

    //DrawRect(0xffff00, 10, 10, 100, 100);

}

extern "C" void _start(stivale2_struct* stivale2_struct) {
    ErebOS os = ErebOS(stivale2_struct);

    for (;;) { asm ("hlt"); }
}