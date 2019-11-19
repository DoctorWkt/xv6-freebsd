void dev_null_init(void);
void dev_zero_init(void);
void dev_random_init(void);
void dev_console_init(void);

void devinit(void)
{
  dev_null_init();
  dev_zero_init();
  dev_random_init();
  dev_console_init();
}
