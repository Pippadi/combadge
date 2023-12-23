$wallThickness = 1;
$wiggleRoom = 0.5;
$chamberDepth = 10;

$pcbMajorAxis = 45;
$pcbMinorAxis = 31;

$chevronHeight = 13;

$fn = 64;

$cavityLength = $pcbMajorAxis + 2*$wiggleRoom;
$cavityWidth = $pcbMinorAxis + 2*$wiggleRoom;
$baseLength = $cavityLength + 2*$wallThickness;
$baseWidth = $cavityWidth + 2*$wallThickness;

module chevron() {
    linear_extrude($chevronHeight) import("chevron.svg");
}

module combadge() {    
    difference() {
        union() {
            translate([6, -2, 0]) scale([0.85, 0.85, 1]) chevron();
            translate([$baseLength/2, $baseWidth/2, 0]) scale([$baseLength/$baseWidth, 1, 1]) union() {
                cylinder($chamberDepth + $wallThickness, d = $baseWidth);
                translate([0, 0, $chamberDepth + $wallThickness]) base_top();
            }
        }
        translate([$baseLength/2, $baseWidth/2, 0]) scale([$cavityLength/$cavityWidth, 1, 1])
            cylinder($chamberDepth, d = $cavityWidth);
    }
}

module base_top() {
    rotate_extrude() polygon(points=[[0, 0], [$baseWidth/2-2, 1], [$baseWidth/2, 0]]);
}

combadge();