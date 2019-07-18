/******************************************************************************
 *
 *  This module implements IBM POWER6-specific AMS extensions
 *
 *  The code has been tested with SLES 11 on different systems.
 *
 *  Written by Michael Perzl (michael@perzl.org)
 *
 *  Version 1.1, October 21, 2013
 *
 *  Version 1.1:  October 21, 2013
 *                - fixed the segmentation faults
 *
 *  Version 1.0:  Oct 12, 2009
 *                - initial version
 *
 ******************************************************************************/

/*
 * The ganglia metric "C" interface, required for building DSO modules.
 */

#include <gm_metric.h>


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "gm_file.h"
#include "libmetrics.h"


#ifndef BUFFSIZE
#define BUFFSIZE 65536
#endif


typedef struct
{
   uint32_t last_read;
   uint32_t thresh;
   char *name;
   char *buffer[BUFFSIZE];
} my_timely_file;


static my_timely_file proc_stat = { 0, 1, "/proc/stat", {0} };
static my_timely_file proc_ppc64_lparcfg = { 0, 0, "/proc/ppc64/lparcfg", {0} };

static time_t boottime = 0;



static char *
my_update_file( my_timely_file *tf )
{
   int now, rval;


   now = time( NULL );
   if (now - tf->last_read > tf->thresh)
   {
      rval = slurpfile( tf->name, tf->buffer, BUFFSIZE );
      if (rval == SYNAPSE_FAILURE)
      {
         err_msg( "my_update_file() got an error from slurpfile() reading %s",
                  tf->name );
         return( (char *) SYNAPSE_FAILURE );
      }
      else
         tf->last_read = now;
   }

   return( tf->buffer[0] );
}



static time_t
boottime_func_CALLED_ONCE( void )
{
   char   *p;
   time_t  boottime;


   p = my_update_file( &proc_stat );

   p = strstr( p, "btime" );
   if (p)
   {
      p = skip_token( p );
      boottime = strtod( p, (char **) NULL );
   }
   else
      boottime = 0;


   return( boottime );
}



g_val_t
ams_hpi_func( void )
{
   g_val_t val;
   static long long hpi_saved = 0;
   long long hpi, hpi_diff;
   static double last_time = 0.0;
   double now, delta_t;
   struct timeval timeValue;
   struct timezone timeZone;
   char *p;


   gettimeofday( &timeValue, &timeZone );

   now = (double) (timeValue.tv_sec - boottime) + (timeValue.tv_usec / 1000000.0);

   p = strstr( my_update_file( &proc_ppc64_lparcfg ), "cmo_fault_time_usec=" );

   if (p)
   {
      delta_t = now - last_time;

      hpi = strtoll( p+20, (char **) NULL, 10 );

      hpi_diff = hpi - hpi_saved;
      if (hpi_diff < 0) 
         hpi_diff = 0LL;

      val.f = (double) (hpi_diff) / delta_t / 1000000.0;

      hpi_saved = hpi;
      last_time = now;
   }
   else
      val.f = -1.0;


   return( val );
}



g_val_t
ams_hpit_func( void )
{
   g_val_t val;
   static long long hpit_saved = 0;
   long long hpit, hpit_diff;
   static double last_time = 0.0;
   double now, delta_t;
   struct timeval timeValue;
   struct timezone timeZone;
   char *p;


   gettimeofday( &timeValue, &timeZone );

   now = (double) (timeValue.tv_sec - boottime) + (timeValue.tv_usec / 1000000.0);

   p = strstr( my_update_file( &proc_ppc64_lparcfg ), "cmo_faults=" );

   if (p)
   {
      delta_t = now - last_time;

      hpit = strtoll( p+11, (char **) NULL, 10 );

      hpit_diff = hpit - hpit_saved;
      if (hpit_diff < 0) 
         hpit_diff = 0LL;

      val.f = (double) (hpit_diff) / delta_t;

      hpit_saved = hpit;
      last_time = now;
   }
   else
      val.f = -1.0;


   return( val );
}



g_val_t
ams_hypv_pagesize_func( void )
{
   g_val_t val;
   char *p;


   p = strstr( my_update_file( &proc_ppc64_lparcfg), "cmo_page_size=" );

   if (p)
      val.int32 = strtol( p+14, (char **) NULL, 10 );
   else
      val.int32 = -1;

   return( val );
}



g_val_t
ams_iohwm_func( void )
{
   g_val_t val;
   char *p;


   p = strstr( my_update_file( &proc_ppc64_lparcfg), "XXXX=" );

   if (p)
      val.d = (double) strtoll( p+14, (char **) NULL, 10 );
   else
      val.d = -1.0;

   return( val );
}



g_val_t
ams_iome_func( void )
{
   g_val_t val;
   char *p;


   p = strstr( my_update_file( &proc_ppc64_lparcfg), "XXXX=" );

   if (p)
      val.d = (double) strtoll( p+14, (char **) NULL, 10 );
   else
      val.d = -1.0;

   return( val );
}



g_val_t
ams_iomu_func( void )
{
   g_val_t val;
   char *p;


   p = strstr( my_update_file( &proc_ppc64_lparcfg), "XXXX=" );

   if (p)
      val.d = (double) strtoll( p+14, (char **) NULL, 10 );
   else
      val.d = -1.0;

   return( val );
}



g_val_t
ams_pmem_func( void )
{
   g_val_t val;
   char *p;


   p = strstr( my_update_file( &proc_ppc64_lparcfg), "backing_memory=" );

   if (p)
      val.d = (double) strtoll( p+15, (char **) NULL, 10 );
   else
      val.d = -1.0;

   return( val );
}



g_val_t
ams_pool_id_func( void )
{
   g_val_t val;
   char *p;


   p = strstr( my_update_file( &proc_ppc64_lparcfg), "entitled_memory_pool_number=" );

   if (p)
      val.int32 = strtol( p+28, (char **) NULL, 10 );
   else
      val.int32 = -1;

   return( val );
}



g_val_t
ams_var_mem_weight_func( void )
{
   g_val_t val;
   char *p;


   p = strstr( my_update_file( &proc_ppc64_lparcfg), "unallocated_entitled_memory_weight=" );

   if (p)
      val.int32 = strtol( p+35, (char **) NULL, 10 );
   else
      val.int32 = -1;

   return( val );
}



/*
 * Declare ourselves so the configuration routines can find and know us.
 * We'll fill it in at the end of the module.
 */
extern mmodule ibmams_module;


static int ibmams_metric_init ( apr_pool_t *p )
{
   int     i;
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

   return( 0 );
}



static void ibmams_metric_cleanup ( void )
{
}



static g_val_t ibmams_metric_handler ( int metric_index )
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
   {0, "ams_hypv_pagesize",  180, GANGLIA_VALUE_INT,    "KB",          "both", "%d",   UDP_HEADER_SIZE+8,  "Hypervisor page size in KB"},
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

