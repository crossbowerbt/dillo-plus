#ifndef __COOKIES_H__
#define __COOKIES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void  a_Cookies_init( void );

#ifdef DISABLE_COOKIES
# define a_Cookies_get_query(url, requester)  dStrdup("")
# define a_Cookies_set()     ;
# define a_Cookies_freeall() ;
# define a_Cookies_get_disabled() ;
# define a_Cookies_set_disabled(bool_t) ;
#else
  char *a_Cookies_get_query(const DilloUrl *query_url,
                            const DilloUrl *requester);
  void  a_Cookies_set(Dlist *cookie_string, const DilloUrl *set_url,
                      const char *server_date);
  void  a_Cookies_freeall( void );

  bool_t  a_Cookies_get_disabled( void );

  void  a_Cookies_set_disabled( bool_t new_state );
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !__COOKIES_H__ */
