#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
/*Структура заголовка IP пакета*/
struct iphdr {
    unsigned int ihl:4;
    unsigned int version:4;
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    unsigned char saddr[4]; /* IP адресс отправителя */
    unsigned char daddr[4]; /* IP адресс получателя */
};

/*Структура буфера данных */
struct my_buf {
    int tlen;
    union tbuffer{
        unsigned char tbuff[1024];
        struct iphdr thdr;
    } tb;
};

struct net_device_stats *stats;
struct my_buf xbf;
int retval, shift, i;
int ssl_stop (struct net_device *dev);
int ssl_open (struct net_device *dev);
static int ssl_transmit(struct sk_buff *skb, struct net_device *dev);
void printbuffer(struct my_buf *txbf);
static void load_pack(struct my_buf *xbf);
int all_transmitted_packages = 0;
int increment = 0;

/*Структура данных драйвера*/
struct net_device ssl_dev = {
    .name              = "ssl",
    .open              = ssl_open,
    .stop              = ssl_stop,
    .hard_start_xmit   = ssl_transmit,
};

static struct net_device_stats *ssl_get_stats(struct net_device *dev){ return dev->priv; }

/*Функция для передачи пакета*/
static int ssl_transmit (struct sk_buff *skb, struct net_device *dev)
{
    struct my_buf *qbf;
    qbf = &xbf;
    if (skb->len > 1024) {
        printk(KERN_WARNING "ssl: T_buffer is small, dropping packet.\n");
        stats->tx_dropped++;
    } 

    else {
        xbf.tlen = skb->len;
        shift = 0;
        while (shift + 1 <= skb->len) {
            xbf.tb.tbuff[shift] = skb->data[shift];
            shift++;
        }
        qbf->tlen = skb->len;
        stats->tx_bytes += skb->len;
        stats->tx_packets++;
    }

    printbuffer(&xbf); //Дамп переданного пакета из буфера
    load_pack(&xbf); //Принимает пакет, который должен быть отправлен
    dev_kfree_skb(skb);
    stats->tx_bytes += skb->len;
    stats->tx_packets++;
    return 0;
}

/*Функция для инициализации интерфейса*/
int ssl_open (struct net_device *dev)
{
    printk(KERN_WARNING "ssl: ssl_open called.\n");
    netif_start_queue (dev);
    return 0;
}

/*Функция для деинициализации интерфейса*/
int ssl_stop (struct net_device *dev)
{
    printk (KERN_WARNING "ssl: ssl_stop called.\n");
    netif_stop_queue(dev);
    return 0;
}

/*Функция для инициализации модуля*/
int ssl_init_module (void)
{
    stats = kmalloc(sizeof(struct net_device_stats), GFP_KERNEL);
    if (stats) {
        memset(stats, 0, sizeof(struct net_device_stats));
        ssl_dev.priv = stats;
        ssl_dev.get_stats = &ssl_get_stats;
    }
    if ((retval = register_netdev (&ssl_dev))) {
        printk (KERN_WARNING "ssl: Error %d while initializing module.\n",retval);
        return retval;
    }
    printk(KERN_WARNING "ssl: initializing the module.\n");
    return 0;
}

/*Функция для выгрузки модуля*/
void ssl_cleanup (void)
{
    unregister_netdev (&ssl_dev);
    printk (KERN_WARNING "ssl: Cleaning Up the Module.\n");
    return;
}

module_init (ssl_init_module);
module_exit (ssl_cleanup);

/*Прочие вспомогательные функции*/
// Печать IP-адреса в формате "разделенный точками".
void printAddr(unsigned char *addr)
{
    int j;
    for (j = 0; j < 4; j++)
    {
    printk("%d", addr[j]);
    if ( j < 3 )
        printk(".");
    }
}

// Печать информации из дампа буфера передатчика   txbf->tlen
void printbuffer(struct my_buf *txbf)
{
    int current_transmitted_packages = txbf->tlen;
    all_transmitted_packages = all_transmitted_packages + current_transmitted_packages;

    printk("\n---TRANSMITING---");

    printk("\n");
    printk("current_transmitted_packages= ");
    printk(current_transmitted_packages);
    printk(" all_transmitted_packages= ");
    printk(all_transmitted_packages);
    
    printk("\n");
    printk("IPv%d: hdr-size=%d pkt-size=%d protocol=%d TTL=%d", txbf->tb.thdr.version, txbf->tb.thdr.ihl*4, ntohs(txbf->tb.thdr.tot_len),txbf->tb.thdr.protocol, txbf->tb.thdr.ttl);
    printk("\n");
    printk("rst= ");
    printAddr(txbf->tb.thdr.saddr);
    printk(" dst= ");
    printAddr(txbf->tb.thdr.daddr);
    printk("\n");
}

// Функция, принимающая пакет, и записывающая его в буффер
static void load_pack(struct my_buf *xbf) {
    struct net_device_stats *stats = ssl_dev.priv;
    struct sk_buff *r_skb;
    r_skb = dev_alloc_skb(xbf->tlen);//аллокирует буфер для приемки пакета
    if (r_skb == NULL) {
        printk(KERN_WARNING "ssl: memory squeeze, dropping packet.\n");
        stats->rx_dropped++;
        return;
    }
    r_skb->dev = &ssl_dev;
    memcpy(skb_put(r_skb, xbf->tlen), xbf->tb.tbuff, xbf->tlen);
    r_skb->protocol=htons(ETH_P_IP);
    netif_rx(r_skb);
    stats->rx_bytes += xbf->tlen;
    stats->rx_packets++;

    increment++;
    printk("\n---RECEIVING---");
    printk("\nincrement=%d", increment);
 }