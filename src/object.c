#include "nanovg.h"
#include "orb.h"

void orb_object_set(orb_data *orb, orb_object *obj, int x, int y, int type)
{
    obj->type = 0;
    obj->x = x;
    obj->y = y;
    obj->type = type;
}

void orb_object_off(NVGcontext *vg, orb_data *orb, orb_object *obj)
{
    double grid_size;
    double line_width;
    double line_width2;

    grid_size = orb_grid_size(orb);
    line_width = grid_size * 0.08;
    line_width2 = line_width * 0.5;
    nvgBeginPath(vg);
    nvgRect(vg, 
        orb_grid_x(orb, obj->x) + line_width2, 
        orb_grid_y(orb, obj->y) + line_width2, 
        grid_size - line_width, 
        grid_size - line_width); 
    nvgClosePath(vg);
    nvgStrokeWidth(vg, line_width);
    nvgStrokeColor(vg, orb->color[0]);
    nvgStroke(vg);
}

void orb_object_avoid(NVGcontext *vg, orb_data *orb, orb_object *obj)
{
    double grid_size;
    double line_width;
    double line_width2;

    grid_size = orb_grid_size(orb);
    line_width = grid_size * 0.08;
    line_width2 = line_width * 0.5;
    nvgBeginPath(vg);
    nvgRect(vg, 
        orb_grid_x(orb, obj->x) + line_width2, 
        orb_grid_y(orb, obj->y) + line_width2, 
        grid_size - line_width, 
        grid_size - line_width); 
    nvgClosePath(vg);
    nvgStrokeWidth(vg, line_width);
    nvgStrokeColor(vg, orb->color[0]);
    nvgStroke(vg);

    nvgBeginPath(vg);
    nvgMoveTo(vg, 
        orb_grid_x(orb, obj->x) + line_width2, 
        orb_grid_y(orb, obj->y) + line_width2);
    nvgLineTo(vg, 
        orb_grid_x(orb, obj->x) + grid_size - line_width2, 
        orb_grid_y(orb, obj->y) + grid_size - line_width2);
    nvgStroke(vg);
    
    nvgBeginPath(vg);
    nvgMoveTo(vg, 
        orb_grid_x(orb, obj->x) + grid_size - line_width2, 
        orb_grid_y(orb, obj->y) + line_width2);
    nvgLineTo(vg, 
        orb_grid_x(orb, obj->x) + line_width2, 
        orb_grid_y(orb, obj->y) + grid_size - line_width2);
    nvgStroke(vg);
}

void orb_object_draw(NVGcontext *vg, orb_data *orb, orb_object *obj)
{
    nvgBeginPath(vg);
    nvgRect(vg, 
        orb_grid_x(orb, obj->x), 
        orb_grid_y(orb, obj->y), 
        orb_grid_size(orb), 
        orb_grid_size(orb)); 
    nvgClosePath(vg);
    nvgFill(vg);
}

void orb_object_list_init(orb_data *orb, orb_object_list *list)
{
    int i;
    list->start = -1;
    list->nobjects = 0;
    list->nextfree = 0;
    list->draw[ORB_SQUARE] = orb_object_draw;
    list->draw[ORB_OFFSQUARE] = orb_object_off;
    list->draw[ORB_AVOIDSQUARE] = orb_object_avoid;
    for(i = 0; i < GRID_SIZE; i++) list->map[i] = -1;
}

void orb_object_list_draw(NVGcontext *vg, orb_data *orb, orb_object_list *list)
{
    int o;
    int id;
    orb_object *obj = list->obj;
    
    id = list->start;
    for(o = 0; o < list->nobjects; o++) {
        list->draw[obj[id].type](vg, orb, &obj[id]); 
        id = obj[id].next;
    }
}

int orb_object_new(orb_data *orb, orb_object_list *list, orb_object **obj)
{
    int id;
    if(list->nobjects > OBJECTS_MAX) return -1;

    id = list->nextfree;
    list->nextfree++;
    *obj = &list->obj[id];

    if(list->start < 0) {
        list->start = id;
    } else {
        (*obj)->next = list->start;
        list->start = id;
    }

    list->nobjects++;

    return id;
}

int orb_object_add_square(orb_data *orb, orb_object_list *list, int x, int y)
{
    orb_object *obj;
    int id;

    id = orb_object_new(orb, list, &obj);
    if(id < 0) return -1;

    orb_object_set(orb, obj, x, y, ORB_SQUARE);
    obj->id = id;
    orb_object_list_map(orb, list, obj);

    return id;
}

int orb_object_add_offsquare(orb_data *orb, orb_object_list *list, int x, int y)
{
    orb_object *obj;
    int id;

    id = orb_object_new(orb, list, &obj);
    if(id < 0) return -1;

    orb_object_set(orb, obj, x, y, ORB_OFFSQUARE);
    obj->id = id;
    orb_object_list_map(orb, list, obj);

    return id;
}

int orb_object_add_avoidsquare(orb_data *orb, orb_object_list *list, int x, int y)
{
    orb_object *obj;
    int id;

    id = orb_object_new(orb, list, &obj);
    if(id < 0) return -1;

    orb_object_set(orb, obj, x, y, ORB_AVOIDSQUARE);
    obj->id = id;
    orb_object_list_map(orb, list, obj);

    return id;
}

void orb_object_list_map(orb_data *orb, orb_object_list *list, orb_object *obj)
{
    int pos;
    pos = orb_grid_pos(orb, obj->x, obj->y);
    list->map[pos] = obj->id;
}

int orb_object_list_get(orb_data *orb, orb_object_list *list,
    orb_object **obj, int pos)
{
    int id;
    orb_object *o;

    id = list->map[pos];
    o = &list->obj[id];
    *obj = o;
    return id;
}

void orb_object_set_type(orb_data *orb, int id, int type)
{
    orb_object_list *list;
    orb_object *obj;
    
    list = &orb->list;
    obj = &list->obj[id];
    obj->type = type;
}

void orb_object_set_fsm_pos(orb_data *orb, int id, uint32_t pos)
{
    orb_object_list *list;
    orb_object *obj;

    list = &orb->list;
    obj = &list->obj[id];
    obj->fsm_pos = pos;
}

void orb_object_set_note(orb_data *orb, int id, int note)
{
    orb_object_list *list;
    orb_object *obj;

    list = &orb->list;
    obj = &list->obj[id];
    obj->note = note;
}

