#include "vmg_ir.hpp"
static constexpr uint32_t TIMEOUT_FOR_SINGLE_ROTATION; //idk
                                                    
class vmg_ir_driver{
    public:

    virtual void update() = 0;
    virtual ~vmg_ir_driver(){}

    vmg_ir_driver(vmg_ir& frontend): ir_front(frontend){}

    void _copy_to_sensor(uint8_t instance);

    private:

    uint32_t last_change_ms;
    uint32_t rotation_count;

    vmg_ir& ir_front;
};
