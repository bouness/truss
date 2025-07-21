#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#define TOL 1e-12
#define MAX_LINE_LENGTH 1024
int decimal_places = 6;

typedef struct {
    int id;
    double x, y, z;
    int constraints[3];
    double loads[3];
} Node;

typedef struct {
    int id;
    int node1, node2;
    double E, A;
} Member;

typedef struct {
    double force;
    double stress;
    double length;
    char status[12];
} MemberResult;

// Dynamic table structures
typedef struct {
    char **data;
    int rows;
    int cols;
    int *col_widths;
} DynamicTable;

double **K_global = NULL;
double *F_global = NULL;
double *displacements = NULL;
double *reactions = NULL;
MemberResult *member_results = NULL;
int n_nodes = 0;
int n_members = 0;
int total_dofs = 0;
Node *nodes = NULL;
Member *members = NULL;

// Function prototypes
void read_input(const char *filename);
void assemble_global_stiffness();
void apply_boundary_conditions();
void compute_reactions();
void compute_member_forces();
void print_results();
void cleanup();
char* trim_whitespace(char *str);
int parse_csv_line(char *line, double *values, int max_values);

// Dynamic table functions
DynamicTable* create_table(int rows, int cols);
void set_table_cell(DynamicTable* table, int row, int col, const char* content);
void calculate_column_widths(DynamicTable* table);
void print_table_border(DynamicTable* table, char left, char middle, char right, char horizontal);
void print_table(DynamicTable* table, const char* title);
void free_table(DynamicTable* table);
char* format_number(double value, int decimal_places);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file> [decimal_places]\n", argv[0]);
        return 1;
    }
    
    if (argc >= 3) {
        decimal_places = atoi(argv[2]);
        if (decimal_places < 1 || decimal_places > 12) {
            printf("Invalid decimal places. Using default 6.\n");
            decimal_places = 6;
        }
    }
    
    read_input(argv[1]);
    assemble_global_stiffness();
    apply_boundary_conditions();
    compute_reactions();
    compute_member_forces();
    print_results();
    cleanup();
    return 0;
}

// Dynamic table implementation
DynamicTable* create_table(int rows, int cols) {
    DynamicTable* table = malloc(sizeof(DynamicTable));
    table->rows = rows;
    table->cols = cols;
    table->col_widths = calloc(cols, sizeof(int));
    
    table->data = malloc(rows * sizeof(char*));
    for (int i = 0; i < rows; i++) {
        table->data[i] = malloc(cols * sizeof(char*));
        for (int j = 0; j < cols; j++) {
            ((char**)table->data[i])[j] = NULL;
        }
    }
    return table;
}

void set_table_cell(DynamicTable* table, int row, int col, const char* content) {
    if (row >= table->rows || col >= table->cols) return;
    
    char** row_data = (char**)table->data[row];
    if (row_data[col]) {
        free(row_data[col]);
    }
    
    if (content) {
        row_data[col] = malloc(strlen(content) + 1);
        strcpy(row_data[col], content);
    } else {
        row_data[col] = malloc(1);
        row_data[col][0] = '\0';
    }
}

void calculate_column_widths(DynamicTable* table) {
    // Initialize all widths to 0
    for (int j = 0; j < table->cols; j++) {
        table->col_widths[j] = 0;
    }
    
    // Find maximum width for each column
    for (int i = 0; i < table->rows; i++) {
        char** row_data = (char**)table->data[i];
        for (int j = 0; j < table->cols; j++) {
            if (row_data[j]) {
                int len = strlen(row_data[j]);
                if (len > table->col_widths[j]) {
                    table->col_widths[j] = len;
                }
            }
        }
    }
    
    // Ensure minimum width of 3 for each column
    for (int j = 0; j < table->cols; j++) {
        if (table->col_widths[j] < 3) {
            table->col_widths[j] = 3;
        }
    }
}

void print_table_border(DynamicTable* table, char left, char middle, char right, char horizontal) {
    printf("%c", left);
    for (int j = 0; j < table->cols; j++) {
        for (int k = 0; k < table->col_widths[j] + 2; k++) {
            printf("%c", horizontal);
        }
        if (j < table->cols - 1) {
            printf("%c", middle);
        }
    }
    printf("%c\n", right);
}

void print_table(DynamicTable* table, const char* title) {
    calculate_column_widths(table);
    
    if (title) {
        printf("\n%s:\n", title);
    }
    
    // Print top border
    print_table_border(table, '+', '+', '+', '-');
    
    // Print each row
    for (int i = 0; i < table->rows; i++) {
        printf("|");
        char** row_data = (char**)table->data[i];
        
        for (int j = 0; j < table->cols; j++) {
            const char* content = row_data[j] ? row_data[j] : "";
            
            // Check if content is numeric (for right alignment)
            int is_numeric = 0;
            if (strlen(content) > 0 && (isdigit(content[0]) || content[0] == '-' || content[0] == '+')) {
                is_numeric = 1;
                for (int k = 1; k < strlen(content); k++) {
                    if (!isdigit(content[k]) && content[k] != '.' && content[k] != 'e' && 
                        content[k] != 'E' && content[k] != '-' && content[k] != '+') {
                        is_numeric = 0;
                        break;
                    }
                }
            }
            
            if (is_numeric) {
                // Right-align numeric content
                printf(" %*s ", table->col_widths[j], content);
            } else {
                // Left-align text content
                printf(" %-*s ", table->col_widths[j], content);
            }
            
            printf("|");
        }
        printf("\n");
        
        // Print separator after header row (first row)
        if (i == 0) {
            print_table_border(table, '+', '+', '+', '-');
        }
    }
    
    // Print bottom border
    print_table_border(table, '+', '+', '+', '-');
}

void free_table(DynamicTable* table) {
    if (!table) return;
    
    for (int i = 0; i < table->rows; i++) {
        char** row_data = (char**)table->data[i];
        for (int j = 0; j < table->cols; j++) {
            if (row_data[j]) {
                free(row_data[j]);
            }
        }
        free(row_data);
    }
    free(table->data);
    free(table->col_widths);
    free(table);
}

char* format_number(double value, int decimal_places) {
    static char buffer[50];
    if (fabs(value) < 1e-10) {
        sprintf(buffer, "0");
    } else {
        sprintf(buffer, "%.*f", decimal_places, value);
    }
    return buffer;
}

void print_results() {
    // Node Displacements Table
    DynamicTable* disp_table = create_table(n_nodes + 1, 4);
    
    // Set headers
    set_table_cell(disp_table, 0, 0, "Node");
    set_table_cell(disp_table, 0, 1, "DX (in)");
    set_table_cell(disp_table, 0, 2, "DY (in)");
    set_table_cell(disp_table, 0, 3, "DZ (in)");
    
    // Set data
    for (int i = 0; i < n_nodes; i++) {
        char node_id[20];
        sprintf(node_id, "%d", nodes[i].id);
        set_table_cell(disp_table, i + 1, 0, node_id);
        set_table_cell(disp_table, i + 1, 1, format_number(displacements[3*i], decimal_places));
        set_table_cell(disp_table, i + 1, 2, format_number(displacements[3*i+1], decimal_places));
        set_table_cell(disp_table, i + 1, 3, format_number(displacements[3*i+2], decimal_places));
    }
    
    print_table(disp_table, "NODE DISPLACEMENTS");
    free_table(disp_table);
    
    // Node Reactions Table - only for constrained nodes
    int constrained_count = 0;
    for (int i = 0; i < n_nodes; i++) {
        if (nodes[i].constraints[0] || nodes[i].constraints[1] || nodes[i].constraints[2]) {
            constrained_count++;
        }
    }
    
    if (constrained_count > 0) {
        DynamicTable* react_table = create_table(constrained_count + 1, 4);
        
        // Set headers
        set_table_cell(react_table, 0, 0, "Node");
        set_table_cell(react_table, 0, 1, "RX (kips)");
        set_table_cell(react_table, 0, 2, "RY (kips)");
        set_table_cell(react_table, 0, 3, "RZ (kips)");
        
        // Set data
        int row = 1;
        for (int i = 0; i < n_nodes; i++) {
            if (nodes[i].constraints[0] || nodes[i].constraints[1] || nodes[i].constraints[2]) {
                char node_id[20];
                sprintf(node_id, "%d", nodes[i].id);
                set_table_cell(react_table, row, 0, node_id);
                set_table_cell(react_table, row, 1, format_number(reactions[3*i], decimal_places));
                set_table_cell(react_table, row, 2, format_number(reactions[3*i+1], decimal_places));
                set_table_cell(react_table, row, 3, format_number(reactions[3*i+2], decimal_places));
                row++;
            }
        }
        
        print_table(react_table, "NODE REACTIONS");
        free_table(react_table);
    }
    
    // Member Forces Table
    if (n_members > 0) {
        DynamicTable* member_table = create_table(n_members + 1, 5);
        
        // Set headers
        set_table_cell(member_table, 0, 0, "Member");
        set_table_cell(member_table, 0, 1, "Force (kips)");
        set_table_cell(member_table, 0, 2, "Stress (ksi)");
        set_table_cell(member_table, 0, 3, "Status");
        set_table_cell(member_table, 0, 4, "Length (in)");
        
        // Set data
        for (int m = 0; m < n_members; m++) {
            char member_id[20];
            sprintf(member_id, "%d", members[m].id);
            set_table_cell(member_table, m + 1, 0, member_id);
            set_table_cell(member_table, m + 1, 1, format_number(member_results[m].force, decimal_places));
            set_table_cell(member_table, m + 1, 2, format_number(member_results[m].stress, decimal_places));
            set_table_cell(member_table, m + 1, 3, member_results[m].status);
            set_table_cell(member_table, m + 1, 4, format_number(member_results[m].length, decimal_places));
        }
        
        print_table(member_table, "MEMBER FORCES AND STRESSES");
        free_table(member_table);
    }
}

char* trim_whitespace(char *str) {
    if (str == NULL) return NULL;
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    *(end+1) = 0;
    return str;
}

int parse_csv_line(char *line, double *values, int max_values) {
    if (line == NULL || values == NULL) return 0;
    char *token;
    int count = 0;
    token = strtok(line, ",");
    while (token != NULL && count < max_values) {
        token = trim_whitespace(token);
        if (strlen(token) > 0) {
            values[count] = atof(token);
            count++;
        }
        token = strtok(NULL, ",");
    }
    return count;
}

// Read input from CSV file
void read_input(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    double values[10];
    int line_num = 0;
    
    // First pass: count nodes and members
    while (fgets(line, sizeof(line), file)) {
        char original_line[MAX_LINE_LENGTH];
        strcpy(original_line, line);
        
        char *cleaned = trim_whitespace(line);
        if (strlen(cleaned) == 0 || cleaned[0] == '#') continue;
        
        // Make a copy for parsing
        char parse_line[MAX_LINE_LENGTH];
        strcpy(parse_line, original_line);
        
        int num_values = parse_csv_line(parse_line, values, 10);
        if (num_values == 10) {
            n_nodes++;
        }
        else if (num_values == 5) {
            n_members++;
        }
        line_num++;
    }
    
    printf("Found %d nodes and %d members\n", n_nodes, n_members);
    
    // Allocate memory
    if (n_nodes > 0) nodes = malloc(n_nodes * sizeof(Node));
    if (n_members > 0) members = malloc(n_members * sizeof(Member));
    
    rewind(file);
    
    int node_count = 0;
    int member_count = 0;
    line_num = 0;
    
    // Second pass: read data
    while (fgets(line, sizeof(line), file)) {
        char original_line[MAX_LINE_LENGTH];
        strcpy(original_line, line);
        
        char *cleaned = trim_whitespace(line);
        if (strlen(cleaned) == 0 || cleaned[0] == '#') continue;
        
        // Make a copy for parsing
        char parse_line[MAX_LINE_LENGTH];
        strcpy(parse_line, original_line);
        
        double values[10];
        int num_values = parse_csv_line(parse_line, values, 10);
        
        if (num_values == 10) {
            Node *n = &nodes[node_count++];
            n->id = (int)values[0];
            n->x = values[1];
            n->y = values[2];
            n->z = values[3];
            n->constraints[0] = (int)values[4];
            n->constraints[1] = (int)values[5];
            n->constraints[2] = (int)values[6];
            n->loads[0] = values[7];
            n->loads[1] = values[8];
            n->loads[2] = values[9];
        }
        else if (num_values == 5) {
            Member *m = &members[member_count++];
            m->id = (int)values[0];
            int node1_id = (int)values[1];
            int node2_id = (int)values[2];
            m->E = values[3];
            m->A = values[4];
            
            // Find node indices
            m->node1 = -1;
            m->node2 = -1;
            for (int j = 0; j < n_nodes; j++) {
                if (nodes[j].id == node1_id) m->node1 = j;
                if (nodes[j].id == node2_id) m->node2 = j;
            }
            if (m->node1 == -1 || m->node2 == -1) {
                fprintf(stderr, "Error: Invalid node ID in member %d (nodes: %d->%d)\n", 
                        m->id, node1_id, node2_id);
                fprintf(stderr, "Valid node IDs: ");
                for (int j = 0; j < n_nodes; j++) {
                    fprintf(stderr, "%d ", nodes[j].id);
                }
                fprintf(stderr, "\n");
                exit(EXIT_FAILURE);
            }
        }
        else {
            fprintf(stderr, "Warning: Skipping line %d with %d values\n", line_num, num_values);
        }
        line_num++;
    }
    
    fclose(file);
    total_dofs = 3 * n_nodes;
    
    printf("Successfully read %d nodes and %d members\n", node_count, member_count);
}

void cleanup() {
    if (K_global) {
        for (int i = 0; i < total_dofs; i++) {
            free(K_global[i]);
        }
        free(K_global);
    }
    free(F_global);
    free(displacements);
    free(reactions);
    free(nodes);
    free(members);
    free(member_results);
}


// Assemble global stiffness matrix
void assemble_global_stiffness() {
    K_global = malloc(total_dofs * sizeof(double *));
    for (int i = 0; i < total_dofs; i++) {
        K_global[i] = calloc(total_dofs, sizeof(double));
    }
    F_global = calloc(total_dofs, sizeof(double));
    
    // Assemble load vector
    for (int i = 0; i < n_nodes; i++) {
        F_global[3*i]   = nodes[i].loads[0];
        F_global[3*i+1] = nodes[i].loads[1];
        F_global[3*i+2] = nodes[i].loads[2];
    }
    
    // Assemble stiffness matrix
    for (int m = 0; m < n_members; m++) {
        int i = members[m].node1;
        int j = members[m].node2;
        
        double dx = nodes[j].x - nodes[i].x;
        double dy = nodes[j].y - nodes[i].y;
        double dz = nodes[j].z - nodes[i].z;
        double L = sqrt(dx*dx + dy*dy + dz*dz);
        
        double cx = dx/L;
        double cy = dy/L;
        double cz = dz/L;
        
        double k = members[m].E * members[m].A / L;
        
        // Element stiffness matrix in global coordinates
        double ke[6][6] = {
            {cx*cx, cx*cy, cx*cz, -cx*cx, -cx*cy, -cx*cz},
            {cy*cx, cy*cy, cy*cz, -cy*cx, -cy*cy, -cy*cz},
            {cz*cx, cz*cy, cz*cz, -cz*cx, -cz*cy, -cz*cz},
            {-cx*cx, -cx*cy, -cx*cz, cx*cx, cx*cy, cx*cz},
            {-cy*cx, -cy*cy, -cy*cz, cy*cx, cy*cy, cy*cz},
            {-cz*cx, -cz*cy, -cz*cz, cz*cx, cz*cy, cz*cz}
        };
        
        for (int p = 0; p < 6; p++) {
            for (int q = 0; q < 6; q++) {
                ke[p][q] *= k;
            }
        }
        
        // Global DOFs for this element
        int dofs[6] = {
            3*i, 3*i+1, 3*i+2,
            3*j, 3*j+1, 3*j+2
        };
        
        // Add to global stiffness matrix
        for (int p = 0; p < 6; p++) {
            for (int q = 0; q < 6; q++) {
                K_global[dofs[p]][dofs[q]] += ke[p][q];
            }
        }
    }
}

void apply_boundary_conditions() {
    // Identify fixed DOFs
    int *is_fixed = calloc(total_dofs, sizeof(int));
    for (int i = 0; i < n_nodes; i++) {
        for (int j = 0; j < 3; j++) {
            if (nodes[i].constraints[j]) {
                is_fixed[3*i + j] = 1;
            }
        }
    }
    
    // Count free DOFs
    int n_free = 0;
    for (int i = 0; i < total_dofs; i++) {
        if (!is_fixed[i]) n_free++;
    }
    
    // Create reduced system
    double **K_reduced = malloc(n_free * sizeof(double *));
    for (int i = 0; i < n_free; i++) {
        K_reduced[i] = calloc(n_free, sizeof(double));
    }
    double *F_reduced = calloc(n_free, sizeof(double));
    
    // Create mapping from full DOF to reduced DOF
    int *dof_map = malloc(total_dofs * sizeof(int));
    int count = 0;
    for (int i = 0; i < total_dofs; i++) {
        if (!is_fixed[i]) {
            dof_map[i] = count++;
        } else {
            dof_map[i] = -1;
        }
    }
    
    // Fill reduced system
    for (int i = 0; i < total_dofs; i++) {
        if (!is_fixed[i]) {
            int i_red = dof_map[i];
            F_reduced[i_red] = F_global[i];
            for (int j = 0; j < total_dofs; j++) {
                if (!is_fixed[j]) {
                    int j_red = dof_map[j];
                    K_reduced[i_red][j_red] = K_global[i][j];
                }
            }
        }
    }
    
    // Solve reduced system using Gaussian elimination
    double *D_reduced = calloc(n_free, sizeof(double));
    
    // Forward elimination with partial pivoting
    for (int piv = 0; piv < n_free; piv++) {
        // Find pivot
        int max_row = piv;
        for (int i = piv + 1; i < n_free; i++) {
            if (fabs(K_reduced[i][piv]) > fabs(K_reduced[max_row][piv])) {
                max_row = i;
            }
        }
        
        // Swap rows
        if (max_row != piv) {
            double *tmp_row = K_reduced[piv];
            K_reduced[piv] = K_reduced[max_row];
            K_reduced[max_row] = tmp_row;
            double tmp_val = F_reduced[piv];
            F_reduced[piv] = F_reduced[max_row];
            F_reduced[max_row] = tmp_val;
        }
        
        // Eliminate
        for (int i = piv + 1; i < n_free; i++) {
            double factor = K_reduced[i][piv] / K_reduced[piv][piv];
            for (int j = piv; j < n_free; j++) {
                K_reduced[i][j] -= factor * K_reduced[piv][j];
            }
            F_reduced[i] -= factor * F_reduced[piv];
        }
    }
    
    // Back substitution
    for (int i = n_free - 1; i >= 0; i--) {
        D_reduced[i] = F_reduced[i];
        for (int j = i + 1; j < n_free; j++) {
            D_reduced[i] -= K_reduced[i][j] * D_reduced[j];
        }
        D_reduced[i] /= K_reduced[i][i];
    }
    
    // Expand to full displacement vector
    displacements = calloc(total_dofs, sizeof(double));
    for (int i = 0; i < total_dofs; i++) {
        if (!is_fixed[i]) {
            displacements[i] = D_reduced[dof_map[i]];
        }
    }
    
    // Cleanup
    for (int i = 0; i < n_free; i++) free(K_reduced[i]);
    free(K_reduced);
    free(F_reduced);
    free(D_reduced);
    free(dof_map);
    free(is_fixed);
}

void compute_reactions() {
    reactions = calloc(total_dofs, sizeof(double));
    
    // R = K * D - F
    for (int i = 0; i < total_dofs; i++) {
        for (int j = 0; j < total_dofs; j++) {
            reactions[i] += K_global[i][j] * displacements[j];
        }
    }
    
    for (int i = 0; i < total_dofs; i++) {
        reactions[i] -= F_global[i];
    }
}

void compute_member_forces() {
    if (n_members == 0) return;
    
    member_results = malloc(n_members * sizeof(MemberResult));
    
    for (int m = 0; m < n_members; m++) {
        int i = members[m].node1;
        int j = members[m].node2;
        
        double dx = nodes[j].x - nodes[i].x;
        double dy = nodes[j].y - nodes[i].y;
        double dz = nodes[j].z - nodes[i].z;
        double L = sqrt(dx*dx + dy*dy + dz*dz);
        
        double cx = dx/L;
        double cy = dy/L;
        double cz = dz/L;
        
        // Get displacements for nodes i and j
        double *disp_i = &displacements[3*i];
        double *disp_j = &displacements[3*j];
        
        // Axial deformation
        double delta = cx*(disp_j[0] - disp_i[0]) +
                       cy*(disp_j[1] - disp_i[1]) +
                       cz*(disp_j[2] - disp_i[2]);
        
        // Member force and stress
        double force = (members[m].E * members[m].A / L) * delta;
        double stress = force / members[m].A;
        
        member_results[m].force = force;
        member_results[m].stress = stress;
        member_results[m].length = L;
        strcpy(member_results[m].status, force > 0 ? "Tension" : "Compression");
    }
}
