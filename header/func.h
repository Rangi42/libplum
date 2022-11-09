struct plum_image * plum_new_image(void);
struct plum_image * plum_copy_image(const struct plum_image * image);
void plum_destroy_image(struct plum_image * image);
struct plum_image * plum_load_image(const void * restrict buffer, size_t size_mode, unsigned long flags, unsigned * restrict error);
struct plum_image * plum_load_image_limited(const void * restrict buffer, size_t size_mode, unsigned long flags, size_t limit, unsigned * restrict error);
size_t plum_store_image(const struct plum_image * image, void * restrict buffer, size_t size_mode, unsigned * restrict error);
unsigned plum_validate_image(const struct plum_image * image);
const char * plum_get_error_text(unsigned error);
const char * plum_get_file_format_name(unsigned format);
uint32_t plum_get_version_number(void);
int plum_check_valid_image_size(uint32_t width, uint32_t height, uint32_t frames);
int plum_check_limited_image_size(uint32_t width, uint32_t height, uint32_t frames, size_t limit);
size_t plum_color_buffer_size(size_t size, unsigned long flags);
size_t plum_pixel_buffer_size(const struct plum_image * image);
size_t plum_palette_buffer_size(const struct plum_image * image);
unsigned plum_rotate_image(struct plum_image * image, unsigned count, int flip);
void plum_convert_colors(void * restrict destination, const void * restrict source, size_t count, unsigned long to, unsigned long from);
uint64_t plum_convert_color(uint64_t color, unsigned long from, unsigned long to);
void plum_remove_alpha(struct plum_image * image);
unsigned plum_sort_palette(struct plum_image * image, unsigned long flags);
unsigned plum_sort_palette_custom(struct plum_image * image, uint64_t (* callback) (void *, uint64_t), void * argument, unsigned long flags);
unsigned plum_reduce_palette(struct plum_image * image);
const uint8_t * plum_validate_palette_indexes(const struct plum_image * image);
int plum_get_highest_palette_index(const struct plum_image * image);
int plum_convert_colors_to_indexes(uint8_t * restrict destination, const void * restrict source, void * restrict palette, size_t count, unsigned long flags);
void plum_convert_indexes_to_colors(void * restrict destination, const uint8_t * restrict source, const void * restrict palette, size_t count, unsigned long flags);
void plum_sort_colors(const void * restrict colors, uint8_t max_index, unsigned long flags, uint8_t * restrict result);
void * plum_malloc(struct plum_image * image, size_t size);
void * plum_calloc(struct plum_image * image, size_t size);
void * plum_realloc(struct plum_image * image, void * buffer, size_t size);
void plum_free(struct plum_image * image, void * buffer);
struct plum_metadata * plum_allocate_metadata(struct plum_image * image, size_t size);
unsigned plum_append_metadata(struct plum_image * image, int type, const void * data, size_t size);
struct plum_metadata * plum_find_metadata(const struct plum_image * image, int type);
