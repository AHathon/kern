int main() 
{
    asm volatile("svc 0xFF");    
    return 0;
}
