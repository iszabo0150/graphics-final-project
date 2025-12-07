## Project 6: Final Project Gear Up

The project handout can be found [here](https://cs1230.graphics/projects/final/gear-up).


### Test Cases

#### L-System Test Cases

**One Stem**
![One Stem](outputs/lsystem/one_stem.png)

**One Branch**
![One Branch](outputs/lsystem/one_branch.png)

**One Iteration**
![One Iteration](outputs/lsystem/one_iteration.png)

**No Rotation**
![No Rotation](outputs/lsystem/no_rotation.png)

**Basic Tree**
![Basic Tree](outputs/lsystem/basic_tree.png)

**From Book**
![From Book](outputs/lsystem/from_book.png)

#### Bump Mapping Test Cases

**Bump Mapping - Cone**
![Bump Cone](outputs/bump/bump_cone.png)
![Bump Cone Min](outputs/bump/bump_cone_min.png)

**Bump Mapping - Cube**
![Bump Cube Min](outputs/bump/bump_cube_min.png)

**Bump Mapping - Cylinder**
![Bump Cylinder](outputs/bump/bump_cyl.png)
![Bump Cylinder Min](outputs/bump/bump_cyl_min.png)

**Bump Mapping - Sphere**
![Bump Sphere](outputs/bump/bump_sphere.png)
![Bump Sphere Min](outputs/bump/bump_sphere_min.png)

**Flat Bump**
![Flat Bump](outputs/bump/flat_bump.png)

**Repeat UV**
![Repeat UV](outputs/bump/repeatUV.png)

**Strength**
![Strength](outputs/bump/strength.png)

#### Normal Mapping Test Cases

**Normal Mapping - Cone**
![Normal Cone](outputs/normal/normal_cone.png)
![Normal Cone Min](outputs/normal/normal_cone_min.png)

**Normal Mapping - Cube**
![Normal Cube](outputs/normal/normal_cube.png)
![Normal Cube Min](outputs/normal/normal_cube_min.png)

**Normal Mapping - Cylinder**
![Normal Cylinder](outputs/normal/normal_cyl.png)
![Normal Cylinder Min](outputs/normal/normal_cyl_min.png)

**Normal Mapping - Sphere**
![Normal Sphere](outputs/normal/normal_sphere.png)
![Normal Sphere Min](outputs/normal/normal_sphere_min.png)

**Flat Normal**
![Flat Normal](outputs/normal/flat_normal.png)

**Repeat UV Normal**
![Repeat UV Normal](outputs/normal/repeatUVNormal.png)

**Strength Normal**
![Strength Normal](outputs/normal/strengthNormal.png)

#### Texture Mapping Test Cases

**Texture - Cone**
![Texture Cone 2](outputs/textures/texture_cone2.png)
![Texture Cone 2 Min](outputs/textures/texture_cone2_min.png)

**Texture - Cube**
![Texture Cube 2](outputs/textures/texture_cube2.png)
![Texture Cube 2 Min](outputs/textures/texture_cube2_min.png)

**Texture - Cylinder**
![Texture Cylinder](outputs/textures/texture_cyl.png)
![Texture Cylinder Min](outputs/textures/texture_cyl_min.png)
![Texture Cylinder 2](outputs/textures/texture_cyl2.png)

**Texture - Sphere**
![Texture Sphere](outputs/textures/texture_sphere.png)
![Texture Sphere 2](outputs/textures/texture_sphere2.png)

### Design Choices

For my L systems, I decided to make a new class that parsed throguh the grammar, LSystem. This class was called within my parser to simply add these new primitives to the primitive array.

For texture mapping, I basically jsut followed th elab !! I copied my UVs from Ray tracing, added them to the VBO and then delt with them within teh fragment shader !

For normal mapping, I added another field to the VBO for the tangent-bitangents of each triangle for each shape. This value was then used to calculate the new normls within the fragment shader

For bump mapping, I followed a very similar proess, expect I first had to calculate the normals from the passed in bump/height maps. I did that within TextureUtils, and the normals thatI did there are stored within a QImage

### Collaboration/References

I used ChatGPT to figure out how to extend the FileReader for all of teh new parameter I ahd to add. I also used it to figure out the regex for the l systems 

I used the sources linked in the Features doc for all of my features! 

### Known Bugs

None that I'm aware of...

### Extra Credit

Overall, my features add up to 160 points ! 2 stars for L-systems, 1 for texture mapping, 2 for normal mapping and 3 for bump mapping !


