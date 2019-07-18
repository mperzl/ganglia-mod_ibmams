/******************************************************************************
 *
 *  This module implements IBM POWER6-specific extensions
 *  for AMS (Active Memory Sharing).
 *
 *  The libperfstat API is used and it can deal with a 32-bit and a 64-bit
 *  kernel and does not require root authority.
 *
 *  The code has been tested with AIX 6.1 TL 03 and higher on different systems.
 *
 *  Written by Michael Perzl (michael@perzl.org)
 *
 *  Version 1.1, Feb 12, 2010
 *
 *  Version 1.1:  Feb 12, 2010
 *                - added checks for possible libperfstat counter resets
 *                - change unit for ams_hypv_pagesize_func() from KB to Bytes
 *
 *  Version 1.0:  Jun 19, 2009
 *                - initial version
 *
 ******************************************************************************/

/*
 * The ganglia metric "C" interface, required for building DSO modules.
 */

#include <gm_metric.h>


#include <stdlib.h>
#include <strings.h>
#include <time.h>

#include <ctype.h>
#include <utmp.h>
#include <stdio.h>
#include <sys/types.h>
#include <procinfo.h>
#include <signal.h>
#include <cf.h>
#include <sys/utsname.h>

#include <libperfstat.h>


static time_t boottime;



g_val_t
ams_hpi_func( void )
{
   g_val_t val;
   perfstat_partition_total_t p;
   static u_longlong_t saved_ams_hpi = 0;
   longlong_t diff;
   static double last_time = 0.0;
   static double last_val = 0.0;
   double now, delta_t;
   struct timeval timeValue;
   struct timezone timeZone;


   gettimeofday( &timeValue, &timeZone );

   now = (double) (timeValue.tv_sec - boottime) + (timeValue.tv_usec / 1000000.0);
 
   if (perfstat_partition_total( NULL, &p, sizeof( perfstat_partition_total_t ), 1 ) == -1)
      val.d = 0.0;
   else
   {
      delta_t = now - last_time;

      if ( delta_t > 0.0 )
      {
         diff = p.hpi - saved_ams_hpi;

         if (diff >= 0LL)
            val.d = (double) diff / delta_t / 1000.0 / 1000.0 / 1000.0;
         else
            val.d = last_val;
      }
      else
         val.d = 0.0;

      saved_ams_hpi = p.hpi;
   }

   last_time = now;
   last_val = val.d;

   return( val );
}



g_val_t
ams_hpit_func( void )
{
   g_val_t val;
   perfstat_partition_total_t p;
   static u_longlong_t saved_ams_hpit = 0;
   longlong_t diff;
   static double last_time = 0.0;
   static double last_val = 0.0;
   double now, delta_t;
   struct timeval timeValue;
   struct timezone timeZone;


   gettimeofday( &timeValue, &timeZone );

   now = (double) (timeValue.tv_sec - boottime) + (timeValue.tv_usec / 1000000.0);
 
   if (perfstat_partition_total( NULL, &p, sizeof( perfstat_partition_total_t ), 1 ) == -1)
      val.d = 0.0;
   else
   {
      delta_t = now - last_time;

      if ( delta_t > 0.0 )
      {
         diff = p.hpit - saved_ams_hpit;

         if (diff >= 0LL)
            val.d = (double) diff / delta_t / 1000.0 / 1000.0 / 1000.0;
         else
            val.d = last_val;
      }
      else
         val.d = 0.0;

      saved_ams_hpit = p.hpit;
   }

   last_time = now;
   last_val = val.d;

   return( val );
}



g_val_t
ams_hypv_pagesize_func( void )
{
   g_val_t val;
   perfstat_partition_total_t p;


   if (perfstat_partition_total( NULL, &p, sizeof( perfstat_partition_total_t ), 1) == -1)
      val.int32 = -1;
   else
      val.int32 = p.hypv_pagesize * 1024;

   return( val );
}



g_val_t
ams_iohwm_func( void )
{
   g_val_t val;
   perfstat_memory_total_t p;


   if (perfstat_memory_total( NULL, &p, sizeof( perfstat_memory_total_t ), 1) == -1)
      val.d = (double) -1.0;
   else
      val.d = (double) p.iohwm;

   return( val );
}



g_val_t
ams_iome_func( void )
{
   g_val_t val;
   perfstat_partition_total_t p;


   if (perfstat_partition_total( NULL, &p, sizeof( perfstat_partition_total_t ), 1) == -1)
      val.d = (double) -1.0;
   else
      val.d = (double) p.iome;

   return( val );
}



g_val_t
ams_iomu_func( void )
{
   g_val_t val;
   perfstat_memory_total_t p;


   if (perfstat_memory_total( NULL, &p, sizeof( perfstat_memory_total_t ), 1) == -1)
      val.d = (double) -1.0;
   else
      val.d = (double) p.iomu;

   return( val );
}



g_val_t
ams_pmem_func( void )
{
   g_val_t val;
   perfstat_partition_total_t p;


   if (perfstat_partition_total( NULL, &p, sizeof( perfstat_partition_total_t ), 1 ) == -1)
      val.d = (double) -1.0;
   else
      val.d = (double) p.pmem;

   return( val );
}



g_val_t
ams_pool_id_func( void )
{
   g_val_t val;
   perfstat_partition_total_t p;


   if (perfstat_partition_total( NULL, &p, sizeof( perfstat_partition_total_t), 1 ) == -1)
      val.int32 = -1;
   else
      val.int32 = p.ams_pool_id;

   return( val );
}



g_val_t
ams_var_mem_weight_func( void )
{
   g_val_t val;
   perfstat_partition_total_t p;


   if (perfstat_partition_total( NULL, &p, sizeof( perfstat_partition_total_t), 1 ) == -1)
      val.int32 = -1;
   else
      val.int32 = p.var_mem_weight;

   return( val );
}



static time_t
boottime_func_CALLED_ONCE( void )
{
   time_t boottime;
   struct utmp buf;
   FILE *utmp;


   utmp = fopen( UTMP_FILE, "r" );

   if (utmp == NULL)
   {
      /* Can't open utmp, use current time as boottime */
      boottime = time( NULL );
   }
   else
   {
      while (fread( (char *) &buf, sizeof( buf ), 1, utmp ) == 1)
      {
         if (buf.ut_type == BOOT_TIME)
         {
            boottime = buf.ut_time;
            break;
        }
      }

      fclose( utmp );
   }

   return( boottime );
}



/*
 * Declare ourselves so the configuration routines can find and know us.
 * We'll fill it in at the end of the module.
 */
extern mmodule ibmams_module;


static int
ibmams_metric_init ( apr_pool_t *p )
{
   int i;
   FILE *f;
   g_val_t val;


   for (i = 0;  ibmams_module.metrics_info[i].name != NULL;  i++)
   {
      /* Initialize the metadata storage for each of the metrics and then
       *  store one or more key/value pairs.  The define MGROUPS defines
       *  the key for the grouping attribute. */
      MMETRIC_INIT_METADATA( &(ibmams_module.metrics_info[i]), p );
      MMETRIC_ADD_METADATA( &(ibmams_module.metrics_info[i]), MGROUP, "ibmams" );
   }


/* initialize the routines which require a time interval */

   boottime = boottime_func_CALLED_ONCE();
   val = ams_hpi_func();
   val = ams_hpit_func();
   val = ams_iomu_func();
   val = ams_pmem_func();

   return( 0 );
}



static void
ibmams_metric_cleanup ( void )
{
}



static g_val_t
ibmams_metric_handler ( int metric_index )
{
   g_val_t val;

/* The metric_index corresponds to the order in which
   the metrics appear in the metric_info array
*/
   switch (metric_index)
   {
      case 0:  return( ams_hpi_func() );
      case 1:  return( ams_hpit_func() );
      case 2:  return( ams_hypv_pagesize_func() );
      case 3:  return( ams_iohwm_func() );
      case 4:  return( ams_iome_func() );
      case 5:  return( ams_iomu_func() );
      case 6:  return( ams_pmem_func() );
      case 7:  return( ams_pool_id_func() );
      case 8:  return( ams_var_mem_weight_func() );
      default: val.uint32 = 0; /* default fallback */
   }

   return( val );
}



static Ganglia_25metric ibmams_metric_info[] = 
{
   {0, "ams_hpi",             15, GANGLIA_VALUE_DOUBLE, "page-ins/sec","both", "%.5f", UDP_HEADER_SIZE+16, "Number of hypervisor page-ins"},
   {0, "ams_hpit",            15, GANGLIA_VALUE_DOUBLE, "seconds",     "both", "%.f",  UDP_HEADER_SIZE+16, "Time spent in hypervisor page-ins (in seconds)"},
   {0, "ams_hypv_pagesize",  180, GANGLIA_VALUE_INT,    "Bytes",       "both", "%d",   UDP_HEADER_SIZE+8,  "Hypervisor page size"},
   {0, "ams_iohwm",           15, GANGLIA_VALUE_DOUBLE, "Bytes",       "both", "%.0f", UDP_HEADER_SIZE+16, "High water mark of I/O memory entitlement used in bytes"},
   {0, "ams_iome",            15, GANGLIA_VALUE_DOUBLE, "Bytes",       "both", "%.0f", UDP_HEADER_SIZE+16, "I/O memory entitlement of the partition in bytes"},
   {0, "ams_iomu",            15, GANGLIA_VALUE_DOUBLE, "Bytes",       "both", "%.0f", UDP_HEADER_SIZE+16, "I/O memory entitlement of the partition in use in bytes"},
   {0, "ams_pmem",            15, GANGLIA_VALUE_DOUBLE, "Bytes",       "both", "%.0f", UDP_HEADER_SIZE+16, "Amount of physical memory currently backing partition's logical memory in bytes"},
   {0, "ams_pool_id",        180, GANGLIA_VALUE_INT,    "",            "both", "%d",   UDP_HEADER_SIZE+8,  "AMS pool id of the pool the LPAR belongs to"},
   {0, "ams_var_mem_weight", 180, GANGLIA_VALUE_INT,    "",            "both", "%d",   UDP_HEADER_SIZE+8,  "Variable memory capacity weight"},
   {0, NULL}
};



mmodule ibmams_module =
{
   STD_MMODULE_STUFF,
   ibmams_metric_init,
   ibmams_metric_cleanup,
   ibmams_metric_info,
   ibmams_metric_handler,
};

