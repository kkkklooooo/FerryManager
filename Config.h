
class Config{
    public:
    Config(int length,int width){
        this->length=length;
        this->width=width;
    }
    int length=50;
    int width=50;
    float Environment_energy_absorb_rate=0.01f;
    float Environment_plant_absorb_rate=0.4f;
    float Environmrnt_step_max_absorb=2;
    float Environment_single_chunk_max_energy=50;
    float Organism_animal_absorb_rate=0.5f;
    float Organism_loss_rate=0.9f;
    float Organism_reproduce_energy_threshold=25;
    float Organism_reproduce_energy_cost=10;
    float Organism_step_energy_cost=1;
    float Orgianism_overlay_param=1.4;
    int Plant_init_radius=3;
    float Animal_energy_rate = 0.3;
};