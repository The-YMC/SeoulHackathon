<?php
    include('dbcon.php');
    include('check.php');

	$user_id = $_SESSION['user_id'];

	try { 
		$stmt = $con->prepare('select * from users where username=:username');
		$stmt->bindParam(':username', $user_id);
		$stmt->execute();

   } catch(PDOException $e) {
		die("Database error: " . $e->getMessage()); 
   }
   
    if (is_login()){
		if ($_SESSION['user_id'] == $user_id && $_SESSION['is_car']==1)
            ;
		else if ($_SESSION['user_id'] == $user_id && $_SESSION['is_car']==0)
            header("Location: carnumber.php");
    }else
        header("Location: index.php"); 
	include('head.php');
?>

<div class="container">
	<div class="page-header">
    	<h1 class="h2">&nbsp; <?php echo $user_id;  ?>의 차량번호 삭제</h1><hr>
    </div>
<div class="row">

    <table class="table table-bordered table-hover table-striped" style="table-layout: fixed">  
        <thead>  
        <tr>  
            <th>차량번호</th>  
            <th>전화번호</th>
            <th>삭제</th>
		<tr>			
		</thead> 
		<?php  
		$stmt = $con->prepare('SELECT carnumber, phonenumber FROM users WHERE username=:username');
		$stmt->bindParam(':username', $user_id);
	    $stmt->execute();
		
            if ($stmt->rowCount() > 0)
            {
                while($row=$stmt->fetch(PDO::FETCH_ASSOC))
	        {
		    extract($row);
        ?>
		<tr>  
			<td><?php echo $carnumber;  ?></td> 
			<td><?php echo $phonenumber; ?></td>
		</td>
			<td><a class="btn btn-warning" href="car_delete.php?del_carnumber=<?php echo $carnumber ?>" onclick="return confirm('<?php echo $carnumber ?> 차량번호를 삭제할까요?')">
			<span class="glyphicon glyphicon-remove"></span>Del</a></td>
			</tr> 
		<?php
                }
             }
        ?>  

        </table>  
</div>

</body>
</html>